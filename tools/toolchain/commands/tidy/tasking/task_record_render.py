from __future__ import annotations

from ....formats.toon.task_codec import render_task_record
from .task_record_parse import extract_snippet
from .task_record_types import TaskRecord


def task_record_to_dict(record: TaskRecord) -> dict:
    payload = {
        "version": record.version,
        "task_id": record.task_id,
        "batch_id": record.batch_id,
        "queue_batch_id": record.batch_id,
        "queue_generation": record.queue_generation,
        "source_file": record.source_file,
        "workspace": record.workspace,
        "source_scope": record.source_scope,
        "compiler_errors": record.summary.compiler_errors,
        "checks": list(record.checks),
        "diagnostics": [],
    }
    if record.source_fingerprint is not None:
        payload["source_fingerprint"] = {
            "mtime_ns": record.source_fingerprint.mtime_ns,
            "size_bytes": record.source_fingerprint.size_bytes,
            "sha256": record.source_fingerprint.sha256,
        }
    snippet_by_diag = {snippet.diagnostic_index: snippet for snippet in record.snippets}
    for index, diagnostic in enumerate(record.diagnostics, 1):
        item = {
            "line": diagnostic.line,
            "col": diagnostic.col,
            "severity": diagnostic.severity,
            "check": diagnostic.check,
            "message": diagnostic.message,
        }
        if diagnostic.file and diagnostic.file != record.source_file:
            item["file"] = diagnostic.file
        if diagnostic.notes:
            item["notes"] = list(diagnostic.notes)

        snippet = snippet_by_diag.get(index)
        if snippet is None:
            snippet = extract_snippet(index, diagnostic)
        if snippet is not None:
            if snippet.source_line is not None:
                item["source_line"] = snippet.source_line
            if snippet.code:
                item["code"] = snippet.code
            if snippet.caret:
                item["caret"] = snippet.caret
            if snippet.notes:
                item["snippet_notes"] = list(snippet.notes)

        payload["diagnostics"].append(item)
    return payload


def render_text(record: TaskRecord) -> str:
    lines: list[str] = [f"File: {record.source_file}"]

    if record.summary.checks:
        checks_text = ", ".join(
            f"{entry.name}({entry.count})" for entry in record.summary.checks
        )
        lines.append(f"Checks: {checks_text}")
    lines.append(f"Diagnostics: {record.summary.diagnostic_count}")
    lines.append(f"CompilerErrors: {str(record.summary.compiler_errors).lower()}")
    lines.append("")

    snippet_by_diag = {snippet.diagnostic_index: snippet for snippet in record.snippets}
    for index, diagnostic in enumerate(record.diagnostics, 1):
        lines.append(
            f"{diagnostic.file}:{diagnostic.line}:{diagnostic.col}: "
            f"{diagnostic.severity}: {diagnostic.message} [{diagnostic.check}]"
        )
        snippet = snippet_by_diag.get(index)
        if snippet is not None and snippet.code:
            prefix = f"{snippet.source_line}" if snippet.source_line is not None else ""
            lines.append(f"    {prefix} | {snippet.code}")
            if snippet.caret:
                lines.append(f"      | {snippet.caret}")
            for note in snippet.notes:
                lines.append(note)
        elif snippet is not None and snippet.caret:
            lines.append(f"      | {snippet.caret}")
            for note in snippet.notes:
                lines.append(note)

        for note in diagnostic.notes:
            if snippet is not None and note in snippet.notes:
                continue
            lines.append(note)
        if index != len(record.diagnostics):
            lines.append("")

    return "\n".join(lines).rstrip() + "\n"


def render_toon(record: TaskRecord) -> str:
    return render_task_record(record)
