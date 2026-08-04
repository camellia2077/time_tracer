from __future__ import annotations

from dataclasses import replace
from pathlib import Path

from ..analyzers import build_diff, ensure_standard_include, load_text_lines
from ..models import (
    ExecutionRecord,
    FixContext,
    FixIntent,
    WorkspaceTextEdit,
    operation_new_name,
    operation_old_name,
    operation_replacement,
)
from ..reasons import CommonReasons
from .text_edit_planner import TextEditPlanner


class TextEditEngine:
    engine_id = "text"

    def __init__(self) -> None:
        self._planner = TextEditPlanner()

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
                record, edits, include_header = self._planner.plan_file_edits(
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

            overlap_ids = self._planner.overlapping_intent_ids(file_edits)
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
