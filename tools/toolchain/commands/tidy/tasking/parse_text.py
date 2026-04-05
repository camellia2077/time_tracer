from __future__ import annotations

from pathlib import Path

from ....formats.toon.task_codec import parse_task_record
from ....services import log_parser
from .task_fingerprint import compute_source_fingerprint
from .task_record_types import (
    TASK_RECORD_VERSION,
    TaskDraft,
    TaskRecord,
)
from .parse_common import (
    batch_id_from_path,
    build_diagnostic,
    build_summary,
    collect_checks,
    extract_snippet,
    flatten_diagnostic_lines,
    task_id_from_path,
)


def build_task_draft(section_lines: list[str]) -> TaskDraft | None:
    diagnostics_raw = log_parser.extract_diagnostics(section_lines)
    return build_task_draft_from_diagnostics(diagnostics_raw, raw_lines=section_lines)


def build_task_draft_from_diagnostics(
    diagnostics_raw: list[dict],
    *,
    raw_lines: list[str] | None = None,
) -> TaskDraft | None:
    if not diagnostics_raw:
        return None

    source_file = str(diagnostics_raw[0].get("file", "")).strip()
    diagnostics = tuple(build_diagnostic(item) for item in diagnostics_raw)
    checks = tuple(collect_checks(diagnostics))
    summary = build_summary(diagnostics, source_file=source_file)
    snippets = tuple(
        extract_snippet(index + 1, diagnostic)
        for index, diagnostic in enumerate(diagnostics)
    )
    filtered_snippets = tuple(snippet for snippet in snippets if snippet is not None)

    return TaskDraft(
        source_file=source_file,
        checks=checks,
        summary=summary,
        diagnostics=diagnostics,
        snippets=filtered_snippets,
        raw_lines=tuple(raw_lines or flatten_diagnostic_lines(diagnostics)),
    )


def finalize_task_record(
    draft: TaskDraft,
    *,
    task_id: str,
    batch_id: str,
    queue_generation: int | None,
    workspace: str,
    source_scope: str | None,
) -> TaskRecord:
    return TaskRecord(
        version=TASK_RECORD_VERSION,
        task_id=task_id,
        batch_id=batch_id,
        queue_generation=queue_generation,
        source_file=draft.source_file,
        source_fingerprint=compute_source_fingerprint(draft.source_file),
        workspace=workspace,
        source_scope=source_scope,
        checks=draft.checks,
        summary=draft.summary,
        diagnostics=draft.diagnostics,
        snippets=draft.snippets,
        raw_lines=draft.raw_lines,
    )


def legacy_task_record_from_text(
    content: str,
    *,
    task_path: Path,
) -> TaskRecord:
    lines = content.splitlines()
    source_file = ""
    for line in lines:
        if line.startswith("File: "):
            source_file = line[len("File: ") :].strip()
            break
    diagnostics_raw = log_parser.extract_diagnostics(lines)
    diagnostics = tuple(build_diagnostic(item) for item in diagnostics_raw)
    if not source_file and diagnostics:
        source_file = diagnostics[0].file
    checks = tuple(collect_checks(diagnostics))
    summary = build_summary(diagnostics, source_file=source_file)
    snippets = tuple(
        snippet
        for snippet in (
            extract_snippet(index + 1, diagnostic)
            for index, diagnostic in enumerate(diagnostics)
        )
        if snippet is not None
    )
    return TaskRecord(
        version=TASK_RECORD_VERSION,
        task_id=task_id_from_path(task_path),
        batch_id=batch_id_from_path(task_path),
        queue_generation=None,
        source_file=source_file,
        source_fingerprint=compute_source_fingerprint(source_file),
        workspace="",
        source_scope=None,
        checks=checks,
        summary=summary,
        diagnostics=diagnostics,
        snippets=snippets,
        raw_lines=tuple(lines),
    )


def toon_task_record_from_text(
    content: str,
    *,
    task_path: Path,
) -> TaskRecord:
    return parse_task_record(content, task_path=task_path)
