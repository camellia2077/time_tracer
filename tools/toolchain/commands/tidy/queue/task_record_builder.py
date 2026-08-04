from __future__ import annotations

from .parse_common import (
    build_diagnostic,
    build_summary,
    collect_checks,
    extract_snippet,
    flatten_diagnostic_lines,
)
from .task_fingerprint import compute_source_fingerprint
from .task_record_types import TASK_RECORD_VERSION, TaskDraft, TaskRecord


def build_task_draft_from_diagnostics(
    diagnostics_raw: list[dict],
    *,
    raw_lines: list[str] | None = None,
) -> TaskDraft | None:
    if not diagnostics_raw:
        return None

    source_file = str(diagnostics_raw[0].get("file", "")).strip()
    diagnostics = tuple(build_diagnostic(item) for item in diagnostics_raw)
    snippets = tuple(
        extract_snippet(index + 1, diagnostic)
        for index, diagnostic in enumerate(diagnostics)
    )
    return TaskDraft(
        source_file=source_file,
        checks=tuple(collect_checks(diagnostics)),
        summary=build_summary(diagnostics, source_file=source_file),
        diagnostics=diagnostics,
        snippets=tuple(snippet for snippet in snippets if snippet is not None),
        raw_lines=tuple(raw_lines or flatten_diagnostic_lines(diagnostics)),
    )


def finalize_task_record(
    draft: TaskDraft,
    *,
    task_id: str,
    cluster_id: str,
    scan_id: str | None = None,
    queue_generation: int | None,
    workspace: str,
    source_scope: str | None,
) -> TaskRecord:
    return TaskRecord(
        version=TASK_RECORD_VERSION,
        task_id=task_id,
        cluster_id=cluster_id,
        scan_id=scan_id,
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
