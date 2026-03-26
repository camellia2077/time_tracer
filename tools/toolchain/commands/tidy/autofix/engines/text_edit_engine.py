from __future__ import annotations

import re
from dataclasses import replace
from pathlib import Path

from ..analyzers import build_diff, ensure_standard_include, load_text_lines, resolve_line_range, select_literal_match
from ..models import ExecutionRecord, FixContext, FixIntent, WorkspaceTextEdit

_REDUNDANT_CAST_PATTERN = re.compile(
    r"static_cast<(?P<target>[^>]+)>\((?P<expr>[^()]+)\)"
)
_DTO_USING_NAMESPACE_LINE = "using namespace tracer_core::core::dto;"


class TextEditEngine:
    engine_id = "text"

    def execute(self, context: FixContext, intents: list[FixIntent]) -> list[ExecutionRecord]:
        records_by_id: dict[str, ExecutionRecord] = {}
        intents_by_file: dict[Path, list[FixIntent]] = {}

        for intent in intents:
            file_path = Path(intent.file_path)
            intents_by_file.setdefault(file_path, []).append(intent)
            records_by_id[intent.intent_id] = ExecutionRecord(
                intent_id=intent.intent_id,
                status="skipped",
                reason="no_edit_generated",
                old_name=str(intent.payload.get("old_name", "")),
                new_name=str(intent.payload.get("new_name", "")),
                replacement=str(intent.payload.get("replacement", "")),
            )

        for file_path, file_intents in intents_by_file.items():
            if not file_path.exists():
                for intent in file_intents:
                    records_by_id[intent.intent_id] = replace(
                        records_by_id[intent.intent_id],
                        status="failed",
                        reason="file_not_found",
                    )
                continue

            loaded = load_text_lines(file_path)
            if loaded is None:
                for intent in file_intents:
                    records_by_id[intent.intent_id] = replace(
                        records_by_id[intent.intent_id],
                        status="failed",
                        reason="file_read_failed",
                    )
                continue
            before_text, lines = loaded

            file_edits: list[tuple[WorkspaceTextEdit, str]] = []
            include_requests: list[FixIntent] = []
            for intent in file_intents:
                if intent.preview_only and not context.dry_run:
                    records_by_id[intent.intent_id] = replace(
                        records_by_id[intent.intent_id],
                        status="skipped",
                        reason="apply_not_enabled_preview_only_rule",
                    )
                    continue
                record, edits, include_header = self._plan_file_edits(
                    intent=intent,
                    file_path=file_path,
                    text=before_text,
                    lines=lines,
                )
                if record.status == "previewed" and not context.dry_run:
                    record = replace(record, status="applied")
                records_by_id[intent.intent_id] = record
                for edit in edits:
                    file_edits.append((edit, intent.intent_id))
                if include_header and record.status in {"previewed", "applied"}:
                    include_requests.append(intent)

            if include_requests:
                include_edit = ensure_standard_include(
                    before_text,
                    file_path=str(file_path),
                    header="cstdint",
                )
                if include_edit is not None:
                    first_intent = include_requests[0]
                    file_edits.append((include_edit, first_intent.intent_id))
                    current = records_by_id[first_intent.intent_id]
                    records_by_id[first_intent.intent_id] = replace(
                        current,
                        edit_count=current.edit_count + 1,
                    )

            overlap_ids = self._overlapping_intent_ids(file_edits)
            if overlap_ids:
                for intent_id in overlap_ids:
                    current = records_by_id[intent_id]
                    records_by_id[intent_id] = replace(
                        current,
                        status="failed",
                        reason="overlapping_workspace_edits",
                        edit_count=0,
                        changed_files=(),
                    )
                file_edits = [item for item in file_edits if item[1] not in overlap_ids]

            file_edits.sort(key=lambda item: (item[0].start_offset, item[0].end_offset), reverse=True)
            after_text = before_text
            for edit, _intent_id in file_edits:
                after_text = (
                    after_text[: edit.start_offset]
                    + edit.new_text
                    + after_text[edit.end_offset :]
                )

            if not context.dry_run and after_text != before_text:
                file_path.write_text(after_text, encoding="utf-8")

            diff = build_diff(file_path, before_text, after_text)
            changed = before_text != after_text
            for intent in file_intents:
                current = records_by_id[intent.intent_id]
                if current.status not in {"previewed", "applied"}:
                    continue
                if changed:
                    records_by_id[intent.intent_id] = replace(
                        current,
                        diff=diff,
                        changed_files=(str(file_path),),
                    )
                else:
                    records_by_id[intent.intent_id] = replace(
                        current,
                        status="skipped",
                        reason="no_edit_generated",
                        edit_count=0,
                        changed_files=(),
                    )

        return [records_by_id[intent.intent_id] for intent in intents]

    def _plan_file_edits(
        self,
        *,
        intent: FixIntent,
        file_path: Path,
        text: str,
        lines: list[str],
    ) -> tuple[ExecutionRecord, list[WorkspaceTextEdit], bool]:
        operation = str(intent.payload.get("operation", "")).strip()
        if operation == "replace_literal_on_line":
            return self._plan_replace_literal_on_line(intent, file_path, text, lines)
        if operation == "replace_redundant_cast_on_line":
            return self._plan_replace_redundant_cast(intent, file_path, text, lines)
        if operation == "insert_prefix_on_line":
            return self._plan_insert_prefix_on_line(intent, file_path, text, lines)
        if operation == "replace_line_with_block":
            return self._plan_replace_line_with_block(intent, file_path, text, lines)
        return (
            ExecutionRecord(
                intent_id=intent.intent_id,
                status="failed",
                reason="unsupported_text_operation",
                old_name=str(intent.payload.get("old_name", "")),
                new_name=str(intent.payload.get("new_name", "")),
                replacement=str(intent.payload.get("replacement", "")),
            ),
            [],
            False,
        )

    def _plan_replace_literal_on_line(
        self,
        intent: FixIntent,
        file_path: Path,
        text: str,
        lines: list[str],
    ) -> tuple[ExecutionRecord, list[WorkspaceTextEdit], bool]:
        line_index = intent.line - 1
        if line_index < 0 or line_index >= len(lines):
            return self._failed_record(intent, "invalid_line"), [], False
        source_line = lines[line_index]
        old_text = str(intent.payload.get("old_name", "")).strip()
        new_text = str(intent.payload.get("new_name") or intent.payload.get("replacement") or "").strip()
        if not old_text or not new_text:
            return self._skipped_record(intent, "missing_runtime_int_payload"), [], False
        match_span = select_literal_match(
            source_line,
            literal=old_text,
            source_index=max(0, intent.col - 1),
        )
        if match_span is None:
            if new_text in source_line and old_text not in source_line:
                return self._skipped_record(intent, "already_rewritten"), [], False
            return self._skipped_record(intent, "no_safe_same_line_runtime_int_match"), [], False
        line_range = resolve_line_range(text, intent.line)
        if line_range is None:
            return self._failed_record(intent, "invalid_line"), [], False
        line_start, _line_end = line_range
        start_col, end_col = match_span
        edit = WorkspaceTextEdit(
            file_path=str(file_path),
            start_offset=line_start + start_col,
            end_offset=line_start + end_col,
            new_text=new_text,
        )
        return (
            ExecutionRecord(
                intent_id=intent.intent_id,
                status="previewed" if True else "previewed",
                reason="google_runtime_int_replaced",
                edit_count=1,
                old_name=old_text,
                new_name=new_text,
                replacement=new_text,
            ),
            [edit],
            bool(intent.payload.get("ensure_include")),
        )

    def _plan_replace_redundant_cast(
        self,
        intent: FixIntent,
        file_path: Path,
        text: str,
        lines: list[str],
    ) -> tuple[ExecutionRecord, list[WorkspaceTextEdit], bool]:
        line_index = intent.line - 1
        if line_index < 0 or line_index >= len(lines):
            return self._failed_record(intent, "invalid_line"), [], False
        source_line = lines[line_index]
        matches = list(_REDUNDANT_CAST_PATTERN.finditer(source_line))
        if not matches:
            return self._skipped_record(intent, "no_safe_same_line_cast_match"), [], False
        source_index = max(0, intent.col - 1)
        selected_match = None
        for match in matches:
            if match.start() <= source_index <= match.end():
                selected_match = match
                break
        if selected_match is None:
            selected_match = matches[0]
        replacement = selected_match.group("expr").strip()
        line_range = resolve_line_range(text, intent.line)
        if line_range is None:
            return self._failed_record(intent, "invalid_line"), [], False
        line_start, _line_end = line_range
        edit = WorkspaceTextEdit(
            file_path=str(file_path),
            start_offset=line_start + selected_match.start(),
            end_offset=line_start + selected_match.end(),
            new_text=replacement,
        )
        return (
            ExecutionRecord(
                intent_id=intent.intent_id,
                status="previewed",
                reason="safe_same_type_cast_removed",
                edit_count=1,
                replacement=replacement,
            ),
            [edit],
            False,
        )

    def _plan_insert_prefix_on_line(
        self,
        intent: FixIntent,
        file_path: Path,
        text: str,
        lines: list[str],
    ) -> tuple[ExecutionRecord, list[WorkspaceTextEdit], bool]:
        line_index = intent.line - 1
        if line_index < 0 or line_index >= len(lines):
            return self._failed_record(intent, "invalid_line"), [], False
        source_line = lines[line_index]
        stripped_line = source_line.lstrip()
        if not stripped_line:
            return self._skipped_record(intent, "empty_constructor_line"), [], False
        prefix = str(intent.payload.get("prefix", ""))
        if stripped_line.startswith(prefix):
            return self._skipped_record(intent, "already_explicit"), [], False
        if "(" not in stripped_line:
            return self._skipped_record(intent, "no_safe_constructor_signature_match"), [], False
        line_range = resolve_line_range(text, intent.line)
        if line_range is None:
            return self._failed_record(intent, "invalid_line"), [], False
        line_start, _line_end = line_range
        indent_len = len(source_line) - len(stripped_line)
        edit = WorkspaceTextEdit(
            file_path=str(file_path),
            start_offset=line_start + indent_len,
            end_offset=line_start + indent_len,
            new_text=prefix,
        )
        return (
            ExecutionRecord(
                intent_id=intent.intent_id,
                status="previewed",
                reason="google_explicit_constructor_added",
                edit_count=1,
                replacement=prefix,
            ),
            [edit],
            False,
        )

    def _plan_replace_line_with_block(
        self,
        intent: FixIntent,
        file_path: Path,
        text: str,
        lines: list[str],
    ) -> tuple[ExecutionRecord, list[WorkspaceTextEdit], bool]:
        line_index = intent.line - 1
        if line_index < 0 or line_index >= len(lines):
            return self._skipped_record(intent, "using_namespace_directive_not_found"), [], False
        source_line = lines[line_index]
        if source_line.strip() != _DTO_USING_NAMESPACE_LINE:
            return self._skipped_record(intent, "using_namespace_directive_not_found"), [], False
        replacement_lines = [
            line for line in str(intent.payload.get("replacement", "")).splitlines() if line.strip()
        ]
        if not replacement_lines:
            return self._skipped_record(intent, "no_safe_using_declarations_generated"), [], False
        line_range = resolve_line_range(text, intent.line)
        if line_range is None:
            return self._skipped_record(intent, "using_namespace_directive_not_found"), [], False
        line_start, line_end = line_range
        indent = source_line[: len(source_line) - len(source_line.lstrip())]
        block_text = "\n".join(indent + line for line in replacement_lines)
        edit = WorkspaceTextEdit(
            file_path=str(file_path),
            start_offset=line_start,
            end_offset=line_end,
            new_text=block_text,
        )
        return (
            ExecutionRecord(
                intent_id=intent.intent_id,
                status="previewed",
                reason="dto_using_declarations_preview",
                edit_count=len(replacement_lines),
                old_name=_DTO_USING_NAMESPACE_LINE,
                replacement="\n".join(replacement_lines),
            ),
            [edit],
            False,
        )

    def _overlapping_intent_ids(
        self,
        edits_with_ids: list[tuple[WorkspaceTextEdit, str]],
    ) -> set[str]:
        overlaps: set[str] = set()
        sorted_items = sorted(edits_with_ids, key=lambda item: (item[0].start_offset, item[0].end_offset))
        previous_end = -1
        previous_intent = ""
        for edit, intent_id in sorted_items:
            if edit.start_offset < previous_end:
                overlaps.add(intent_id)
                if previous_intent:
                    overlaps.add(previous_intent)
            previous_end = max(previous_end, edit.end_offset)
            previous_intent = intent_id
        return overlaps

    @staticmethod
    def _failed_record(intent: FixIntent, reason: str) -> ExecutionRecord:
        return ExecutionRecord(
            intent_id=intent.intent_id,
            status="failed",
            reason=reason,
            old_name=str(intent.payload.get("old_name", "")),
            new_name=str(intent.payload.get("new_name", "")),
            replacement=str(intent.payload.get("replacement", "")),
        )

    @staticmethod
    def _skipped_record(intent: FixIntent, reason: str) -> ExecutionRecord:
        return ExecutionRecord(
            intent_id=intent.intent_id,
            status="skipped",
            reason=reason,
            old_name=str(intent.payload.get("old_name", "")),
            new_name=str(intent.payload.get("new_name", "")),
            replacement=str(intent.payload.get("replacement", "")),
        )
