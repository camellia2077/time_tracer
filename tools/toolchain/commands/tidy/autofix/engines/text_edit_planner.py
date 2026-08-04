from __future__ import annotations

from pathlib import Path

from ..analyzers import resolve_line_range, select_literal_match
from ..models import (
    ExecutionRecord,
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
from ..rules.braces_around_statements import build_braced_statement


class TextEditPlanner:
    """Create safe same-file edits without reading or writing the workspace."""

    def plan_file_edits(
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

    def overlapping_intent_ids(
        self,
        edits_with_ids: list[tuple[WorkspaceTextEdit, str]],
    ) -> set[str]:
        overlaps: set[str] = set()
        sorted_items = sorted(
            edits_with_ids,
            key=lambda item: (item[0].start_offset, item[0].end_offset),
        )
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
        replacement_lines = list(operation.replacement_lines)
        if source_line.strip() != operation.expected_line.strip():
            if operation.match_mode != "braces_single_line":
                return self._skipped_record(intent, operation.missing_reason), [], None
            dynamic_replacement = build_braced_statement(source_line, ("{",))
            if dynamic_replacement is None:
                return self._skipped_record(intent, operation.missing_reason), [], None
            replacement_lines = list(dynamic_replacement)
        replacement_lines = [line for line in replacement_lines if line.strip()]
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
