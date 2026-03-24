from __future__ import annotations

from .task_record_parse import extract_snippet
from .task_record_types import TaskRecord


def task_record_to_dict(record: TaskRecord) -> dict:
    payload = {
        "version": record.version,
        "task_id": record.task_id,
        "batch_id": record.batch_id,
        "source_file": record.source_file,
        "workspace": record.workspace,
        "source_scope": record.source_scope,
        "compiler_errors": record.summary.compiler_errors,
        "checks": list(record.checks),
        "diagnostics": [],
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
    lines = [
        "task:",
        f"  id: {record.task_id}",
        f"  batch: {record.batch_id}",
        f"  source: {record.source_file}",
        f"  workspace: {record.workspace or '<unset>'}",
    ]
    if record.source_scope:
        lines.append(f"  source_scope: {record.source_scope}")
    lines.extend(
        [
            "summary:",
            f"  diagnostics: {record.summary.diagnostic_count}",
            f"  compiler_errors: {str(record.summary.compiler_errors).lower()}",
        ]
    )
    if record.summary.checks:
        check_rows = ",".join(f"{entry.name},{entry.count}" for entry in record.summary.checks)
        lines.append(f"checks[{len(record.summary.checks)}]{{name,count}}:")
        lines.append(f"  {check_rows}" if len(record.summary.checks) == 1 else "")
        if len(record.summary.checks) > 1:
            for entry in record.summary.checks:
                lines.append(f"  {entry.name},{entry.count}")
    if record.diagnostics:
        lines.append(
            f"diagnostics[{len(record.diagnostics)}]"
            "{index,line,col,severity,check,message}:"
        )
        for index, diagnostic in enumerate(record.diagnostics, 1):
            message = diagnostic.message.replace("\n", " ").replace(",", ";")
            lines.append(
                f"  {index},{diagnostic.line},{diagnostic.col},"
                f"{diagnostic.severity},{diagnostic.check},{message}"
            )
    if record.snippets:
        lines.append(f"snippets[{len(record.snippets)}]{{diag,line,code,caret}}:")
        for snippet in record.snippets:
            source_line = "" if snippet.source_line is None else str(snippet.source_line)
            code = snippet.code.replace("\n", " ").replace(",", ";")
            caret = snippet.caret.replace("\n", " ").replace(",", ";")
            lines.append(f"  {snippet.diagnostic_index},{source_line},{code},{caret}")
    return "\n".join(line for line in lines if line != "") + "\n"
