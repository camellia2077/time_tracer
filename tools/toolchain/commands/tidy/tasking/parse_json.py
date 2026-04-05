from __future__ import annotations

from pathlib import Path

from .task_record_types import (
    TASK_RECORD_VERSION,
    TaskDiagnostic,
    TaskRecord,
    TaskSnippet,
    TaskSummary,
)
from .parse_common import (
    batch_id_from_path,
    build_summary,
    collect_checks,
    compose_raw_lines,
    flatten_diagnostic_lines,
    read_dict_list,
    read_int,
    read_optional_int,
    read_optional_text,
    read_source_fingerprint,
    read_str_list,
    read_text,
    task_id_from_path,
)


def task_record_from_dict(payload: dict, *, fallback_path: Path | None = None) -> TaskRecord:
    task_id = read_text(payload.get("task_id")) or task_id_from_path(fallback_path)
    batch_id = (
        read_text(payload.get("queue_batch_id"))
        or read_text(payload.get("batch_id"))
        or batch_id_from_path(fallback_path)
    )
    queue_generation = read_optional_int(payload.get("queue_generation"))
    source_file = read_text(payload.get("source_file"))
    source_fingerprint = read_source_fingerprint(payload.get("source_fingerprint"))
    diagnostics_payload = read_dict_list(payload.get("diagnostics"))

    diagnostics_list = []
    snippets_list: list[TaskSnippet] = []
    for index, item in enumerate(diagnostics_payload, 1):
        diag_file = read_text(item.get("file")) or source_file
        line = read_int(item.get("line"))
        col = read_int(item.get("col"))
        severity = read_text(item.get("severity"))
        check = read_text(item.get("check"))
        message = read_text(item.get("message"))
        notes = tuple(read_str_list(item.get("notes")))
        snippet_notes = tuple(read_str_list(item.get("snippet_notes")))
        source_line = read_optional_int(item.get("source_line"))
        code = read_text(item.get("code"))
        caret = read_text(item.get("caret"))
        raw_lines_payload = read_str_list(item.get("raw_lines"))
        if raw_lines_payload:
            raw_lines = tuple(raw_lines_payload)
        else:
            raw_lines = compose_raw_lines(
                file=diag_file,
                line=line,
                col=col,
                severity=severity,
                check=check,
                message=message,
                source_line=source_line,
                code=code,
                caret=caret,
                snippet_notes=snippet_notes,
                notes=notes,
            )
        diagnostics_list.append(
            TaskDiagnostic(
                file=diag_file,
                line=line,
                col=col,
                severity=severity,
                check=check,
                message=message,
                raw_lines=raw_lines,
                notes=notes,
            )
        )
        if source_line is not None or code or caret or snippet_notes:
            snippets_list.append(
                TaskSnippet(
                    diagnostic_index=index,
                    source_line=source_line,
                    code=code,
                    caret=caret,
                    notes=snippet_notes,
                )
            )

    diagnostics = tuple(diagnostics_list)
    snippets = tuple(snippets_list)
    if not source_file and diagnostics:
        source_file = diagnostics[0].file

    checks = tuple(read_str_list(payload.get("checks")))
    if not checks:
        checks = tuple(collect_checks(diagnostics))
    inferred_summary = build_summary(diagnostics, source_file=source_file)
    compiler_errors = bool(payload.get("compiler_errors", inferred_summary.compiler_errors))
    summary = TaskSummary(
        diagnostic_count=len(diagnostics),
        compiler_errors=compiler_errors,
        files=inferred_summary.files,
        checks=inferred_summary.checks,
    )

    return TaskRecord(
        version=read_int(payload.get("version"), default=TASK_RECORD_VERSION),
        task_id=task_id,
        batch_id=batch_id,
        queue_generation=queue_generation,
        source_file=source_file,
        source_fingerprint=source_fingerprint,
        workspace=read_text(payload.get("workspace")),
        source_scope=read_optional_text(payload.get("source_scope")),
        checks=checks,
        summary=summary,
        diagnostics=diagnostics,
        snippets=snippets,
        raw_lines=tuple(flatten_diagnostic_lines(diagnostics)),
    )
