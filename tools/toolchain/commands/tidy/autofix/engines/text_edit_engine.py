from __future__ import annotations

from dataclasses import replace
from pathlib import Path

from ..analyzers import build_diff, ensure_standard_include, load_text_lines, resolve_line_range, select_literal_match
from ..models import (
    ExecutionRecord,
    FixContext,
    FixIntent,
    InsertPrefixOnLineOp,
    ReplaceLineWithBlockOp,
    ReplaceLiteralOnLineOp,
    WorkspaceTextEdit,
    operation_new_name,
    operation_old_name,
    operation_replacement,
)
from ..reasons import CommonReasons


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
                reason=CommonReasons.NO_EDIT_GENERATED,
                old_name=operation_old_name(intent.operation),
                new_name=operation_new_name(intent.operation),
                replacement=operation_replacement(intent.operation),
            )

        for file_path, file_intents in intents_by_file.items():
            if not file_path.exists():
                for intent in file_intents:
                    records_by_id[intent.intent_id] = replace(
                        records_by_id[intent.intent_id],
                        status="failed",
                        reason=CommonReasons.FILE_NOT_FOUND,
                    )
                continue

            loaded = load_text_lines(file_path)
            if loaded is None:
                for intent in file_intents:
                    records_by_id[intent.intent_id] = replace(
                        records_by_id[intent.intent_id],
                        status="failed",
                        reason=CommonReasons.FILE_READ_FAILED,
                    )
                continue
            before_text, lines = loaded

            file_edits: list[tuple[WorkspaceTextEdit, str]] = []
            include_requests: list[tuple[str, FixIntent]] = []
            for intent in file_intents:
                if intent.preview_only and not context.dry_run:
                    records_by_id[intent.intent_id] = replace(
                        records_by_id[intent.intent_id],
                        status="skipped",
                        reason=CommonReasons.APPLY_NOT_ENABLED_PREVIEW_ONLY_RULE,
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
                    include_requests.append((include_header, intent))

            if include_requests:
                applied_headers: set[str] = set()
                for header, include_intent in include_requests:
                    if header in applied_headers:
                        continue
                    include_edit = ensure_standard_include(
                        before_text,
                        file_path=str(file_path),
                        header=header,
                    )
                    if include_edit is None:
                        continue
                    file_edits.append((include_edit, include_intent.intent_id))
                    current = records_by_id[include_intent.intent_id]
                    records_by_id[include_intent.intent_id] = replace(
                        current,
                        edit_count=current.edit_count + 1,
                    )
                    applied_headers.add(header)

            overlap_ids = self._overlapping_intent_ids(file_edits)
            if overlap_ids:
                for intent_id in overlap_ids:
                    current = records_by_id[intent_id]
                    records_by_id[intent_id] = replace(
                        current,
                        status="failed",
                        reason=CommonReasons.OVERLAPPING_WORKSPACE_EDITS,
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
                        reason=CommonReasons.NO_EDIT_GENERATED,
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
    ) -> tuple[ExecutionRecord, list[WorkspaceTextEdit], str | None]:
        operation = intent.operation
        if isinstance(operation, ReplaceLiteralOnLineOp):
            return self._plan_replace_literal_on_line(intent, file_path, text, lines, operation)
        if isinstance(operation, InsertPrefixOnLineOp):
            return self._plan_insert_prefix_on_line(intent, file_path, text, lines, operation)
        if isinstance(operation, ReplaceLineWithBlockOp):
            return self._plan_replace_line_with_block(intent, file_path, text, lines, operation)
        return (
            ExecutionRecord(
                intent_id=intent.intent_id,
                status="failed",
                reason=CommonReasons.UNSUPPORTED_TEXT_OPERATION,
                old_name=operation_old_name(intent.operation),
                new_name=operation_new_name(intent.operation),
                replacement=operation_replacement(intent.operation),
            ),
            [],
            None,
        )

    def _plan_replace_literal_on_line(
        self,
        intent: FixIntent,
        file_path: Path,
        text: str,
        lines: list[str],
        operation: ReplaceLiteralOnLineOp,
    ) -> tuple[ExecutionRecord, list[WorkspaceTextEdit], str | None]:
        line_index = intent.line - 1
        if line_index < 0 or line_index >= len(lines):
            return self._failed_record(intent, CommonReasons.INVALID_LINE), [], None
        source_line = lines[line_index]
        old_text = operation.old_name.strip()
        new_text = operation.new_name.strip()
        if not old_text or not new_text:
            return self._skipped_record(intent, operation.missing_reason), [], None
        match_span = select_literal_match(
            source_line,
            literal=old_text,
            source_index=max(0, intent.col - 1),
        )
        if match_span is None:
            if new_text in source_line and old_text not in source_line:
                return self._skipped_record(intent, operation.already_rewritten_reason), [], None
            return self._skipped_record(intent, operation.no_match_reason), [], None
        line_range = resolve_line_range(text, intent.line)
        if line_range is None:
            return self._failed_record(intent, CommonReasons.INVALID_LINE), [], None
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
                status="previewed",
                reason=operation.success_reason,
                edit_count=1,
                old_name=old_text,
                new_name=new_text,
                replacement=new_text,
            ),
            [edit],
            operation.ensure_include.strip() or None,
        )

    def _plan_insert_prefix_on_line(
        self,
        intent: FixIntent,
        file_path: Path,
        text: str,
        lines: list[str],
        operation: InsertPrefixOnLineOp,
    ) -> tuple[ExecutionRecord, list[WorkspaceTextEdit], str | None]:
        line_index = intent.line - 1
        if line_index < 0 or line_index >= len(lines):
            return self._failed_record(intent, CommonReasons.INVALID_LINE), [], None
        source_line = lines[line_index]
        stripped_line = source_line.lstrip()
        if not stripped_line:
            return self._skipped_record(intent, operation.no_match_reason), [], None
        if stripped_line.startswith(operation.prefix):
            return self._skipped_record(intent, operation.already_prefixed_reason), [], None
        if "(" not in stripped_line:
            return self._skipped_record(intent, operation.no_match_reason), [], None
        line_range = resolve_line_range(text, intent.line)
        if line_range is None:
            return self._failed_record(intent, CommonReasons.INVALID_LINE), [], None
        line_start, _line_end = line_range
        indent_len = len(source_line) - len(stripped_line)
        edit = WorkspaceTextEdit(
            file_path=str(file_path),
            start_offset=line_start + indent_len,
            end_offset=line_start + indent_len,
            new_text=operation.prefix,
        )
        return (
            ExecutionRecord(
                intent_id=intent.intent_id,
                status="previewed",
                reason=operation.success_reason,
                edit_count=1,
                replacement=operation.prefix,
            ),
            [edit],
            None,
        )

    def _plan_replace_line_with_block(
        self,
        intent: FixIntent,
        file_path: Path,
        text: str,
        lines: list[str],
        operation: ReplaceLineWithBlockOp,
    ) -> tuple[ExecutionRecord, list[WorkspaceTextEdit], str | None]:
        line_index = intent.line - 1
        if line_index < 0 or line_index >= len(lines):
            return self._skipped_record(intent, operation.missing_reason), [], None
        source_line = lines[line_index]
        if source_line.strip() != operation.expected_line.strip():
            return self._skipped_record(intent, operation.missing_reason), [], None
        replacement_lines = [line for line in operation.replacement_lines if line.strip()]
        if not replacement_lines:
            return self._skipped_record(intent, operation.empty_replacement_reason), [], None
        line_range = resolve_line_range(text, intent.line)
        if line_range is None:
            return self._skipped_record(intent, operation.missing_reason), [], None
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
                reason=operation.success_reason,
                edit_count=len(replacement_lines),
                old_name=operation.expected_line,
                replacement="\n".join(replacement_lines),
            ),
            [edit],
            None,
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
            old_name=operation_old_name(intent.operation),
            new_name=operation_new_name(intent.operation),
            replacement=operation_replacement(intent.operation),
        )

    @staticmethod
    def _skipped_record(intent: FixIntent, reason: str) -> ExecutionRecord:
        return ExecutionRecord(
            intent_id=intent.intent_id,
            status="skipped",
            reason=reason,
            old_name=operation_old_name(intent.operation),
            new_name=operation_new_name(intent.operation),
            replacement=operation_replacement(intent.operation),
        )
