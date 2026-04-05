from __future__ import annotations

from collections import defaultdict
from pathlib import Path
import re

from ...commands.tidy.tasking.task_record_types import (
    TASK_RECORD_VERSION,
    SourceFingerprint,
    TaskDiagnostic,
    TaskRecord,
    TaskSnippet,
    TaskSummary,
    TaskSummaryEntry,
)
from .base import escape_cell, split_row

_TASK_ARTIFACT_PATTERN = re.compile(r"^task_(\d+)\.(?:json|log|toon)$")
_VISUAL_MARKER_CHARS = frozenset("~^.-")


def render_task_record(record: TaskRecord) -> str:
    lines = [
        "task:",
        f"  id: {record.task_id}",
        f"  batch: {record.batch_id}",
        f"  source: {record.source_file}",
        f"  workspace: {record.workspace or '<unset>'}",
    ]
    if record.queue_generation is not None:
        lines.append(f"  queue_generation: {record.queue_generation}")
    if record.source_scope:
        lines.append(f"  source_scope: {record.source_scope}")
    if record.source_fingerprint is not None:
        lines.append(f"  source_mtime_ns: {record.source_fingerprint.mtime_ns}")
        lines.append(f"  source_size_bytes: {record.source_fingerprint.size_bytes}")
        lines.append(f"  source_sha256: {record.source_fingerprint.sha256}")
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
            lines.append(
                f"  {index},{diagnostic.line},{diagnostic.col},"
                f"{diagnostic.severity},{diagnostic.check},{escape_cell(diagnostic.message)}"
            )

    snippet_rows: list[tuple[int, str, str, str]] = []
    for snippet in record.snippets:
        hint = _toon_semantic_hint(snippet.caret)
        if snippet.source_line is None and not snippet.code and not hint:
            continue
        source_line = "" if snippet.source_line is None else str(snippet.source_line)
        code = escape_cell(snippet.code, trim=False)
        snippet_rows.append((snippet.diagnostic_index, source_line, code, hint))
    if snippet_rows:
        lines.append(f"snippets[{len(snippet_rows)}]{{diag,line,code,hint}}:")
        for diagnostic_index, source_line, code, hint in snippet_rows:
            lines.append(f"  {diagnostic_index},{source_line},{code},{hint}")

    return "\n".join(line for line in lines if line != "") + "\n"


def parse_task_record(content: str, *, task_path: Path) -> TaskRecord:
    lines = content.splitlines()
    section = ""
    task_id = _task_id_from_path(task_path)
    batch_id = _batch_id_from_path(task_path)
    source_file = ""
    workspace = ""
    queue_generation: int | None = None
    source_scope: str | None = None
    source_mtime_ns: int | None = None
    source_size_bytes: int | None = None
    source_sha256 = ""
    compiler_errors = False
    diagnostic_rows: list[dict] = []
    snippet_rows: dict[int, dict] = {}

    for raw_line in lines:
        line = raw_line.rstrip()
        stripped = line.strip()
        if not stripped:
            continue
        if stripped == "task:":
            section = "task"
            continue
        if stripped == "summary:":
            section = "summary"
            continue
        if stripped.startswith("checks["):
            section = "checks"
            continue
        if stripped.startswith("diagnostics["):
            section = "diagnostics"
            continue
        if stripped.startswith("snippets["):
            section = "snippets"
            continue
        if not line.startswith("  "):
            continue

        value = line[2:]
        if section == "task" and ":" in value:
            key, item = value.split(":", 1)
            parsed = item.strip()
            if key == "id":
                task_id = parsed or task_id
            elif key == "batch":
                batch_id = parsed or batch_id
            elif key == "source":
                source_file = parsed
            elif key == "workspace":
                workspace = "" if parsed == "<unset>" else parsed
            elif key == "queue_generation":
                queue_generation = _read_optional_int(parsed)
            elif key == "source_scope":
                source_scope = parsed or None
            elif key == "source_mtime_ns":
                source_mtime_ns = _read_optional_int(parsed)
            elif key == "source_size_bytes":
                source_size_bytes = _read_optional_int(parsed)
            elif key == "source_sha256":
                source_sha256 = parsed
            continue
        if section == "summary" and ":" in value:
            key, item = value.split(":", 1)
            parsed = item.strip()
            if key == "compiler_errors":
                compiler_errors = parsed.lower() == "true"
            continue
        if section == "diagnostics":
            parts = split_row(value, expected_parts=6, maxsplit=5)
            if parts is None:
                continue
            diagnostic_rows.append(
                {
                    "index": _read_int(parts[0], default=len(diagnostic_rows) + 1),
                    "line": _read_int(parts[1]),
                    "col": _read_int(parts[2]),
                    "severity": parts[3],
                    "check": parts[4],
                    "message": parts[5],
                }
            )
            continue
        if section == "snippets":
            parts = split_row(value, expected_parts=4, maxsplit=3)
            if parts is None:
                continue
            snippet_rows[_read_int(parts[0], default=0)] = {
                "source_line": _read_optional_int(parts[1]),
                "code": parts[2],
                "hint": parts[3],
            }

    diagnostics_list: list[TaskDiagnostic] = []
    snippets_list: list[TaskSnippet] = []
    for item in diagnostic_rows:
        index = int(item["index"])
        snippet_item = snippet_rows.get(index, {})
        hint_text = str(snippet_item.get("hint", ""))
        snippet = TaskSnippet(
            diagnostic_index=index,
            source_line=snippet_item.get("source_line"),
            code=str(snippet_item.get("code", "")),
            caret=hint_text,
            notes=(),
        )
        if snippet.source_line is not None or snippet.code or snippet.caret:
            snippets_list.append(snippet)
        diagnostics_list.append(
            TaskDiagnostic(
                file=source_file,
                line=int(item["line"]),
                col=int(item["col"]),
                severity=str(item["severity"]),
                check=str(item["check"]),
                message=str(item["message"]),
                raw_lines=_compose_raw_lines(
                    file=source_file,
                    line=int(item["line"]),
                    col=int(item["col"]),
                    severity=str(item["severity"]),
                    check=str(item["check"]),
                    message=str(item["message"]),
                    source_line=snippet.source_line,
                    code=snippet.code,
                    caret=snippet.caret,
                ),
                notes=(),
            )
        )

    diagnostics = tuple(diagnostics_list)
    summary = _build_summary(
        diagnostics,
        source_file=source_file,
        compiler_errors=compiler_errors,
    )
    return TaskRecord(
        version=TASK_RECORD_VERSION,
        task_id=task_id,
        batch_id=batch_id,
        queue_generation=queue_generation,
        source_file=source_file,
        source_fingerprint=_build_source_fingerprint(
            source_mtime_ns=source_mtime_ns,
            source_size_bytes=source_size_bytes,
            source_sha256=source_sha256,
        ),
        workspace=workspace,
        source_scope=source_scope,
        checks=tuple(entry.name for entry in summary.checks),
        summary=summary,
        diagnostics=diagnostics,
        snippets=tuple(snippets_list),
        raw_lines=tuple(_flatten_diagnostic_lines(diagnostics)),
    )


def _toon_semantic_hint(text: str) -> str:
    normalized = escape_cell(text)
    if not normalized or _is_visual_marker(normalized):
        return ""
    return normalized


def _is_visual_marker(text: str) -> bool:
    stripped = text.strip()
    return bool(stripped) and all(char in _VISUAL_MARKER_CHARS for char in stripped)


def _build_summary(
    diagnostics: tuple[TaskDiagnostic, ...],
    *,
    source_file: str,
    compiler_errors: bool,
) -> TaskSummary:
    file_counts: defaultdict[str, int] = defaultdict(int)
    check_counts: defaultdict[str, int] = defaultdict(int)
    has_compiler_errors = compiler_errors

    if source_file:
        file_counts[source_file] += 0

    for diagnostic in diagnostics:
        if diagnostic.file:
            file_counts[diagnostic.file] += 1
        if diagnostic.check:
            check_counts[diagnostic.check] += 1
        if diagnostic.severity == "error" or diagnostic.check == "clang-diagnostic-error":
            has_compiler_errors = True

    return TaskSummary(
        diagnostic_count=len(diagnostics),
        compiler_errors=has_compiler_errors,
        files=tuple(
            TaskSummaryEntry(name=name, count=count)
            for name, count in sorted(file_counts.items(), key=lambda item: (-item[1], item[0]))
        ),
        checks=tuple(
            TaskSummaryEntry(name=name, count=count)
            for name, count in sorted(check_counts.items(), key=lambda item: (-item[1], item[0]))
        ),
    )


def _compose_raw_lines(
    *,
    file: str,
    line: int,
    col: int,
    severity: str,
    check: str,
    message: str,
    source_line: int | None,
    code: str,
    caret: str,
) -> tuple[str, ...]:
    lines = [f"{file}:{line}:{col}: {severity}: {message} [{check}]"]
    if code:
        if source_line is None:
            lines.append(f"      | {code}")
        else:
            lines.append(f"    {source_line} | {code}")
    if caret:
        lines.append(f"      | {caret}")
    return tuple(lines)


def _flatten_diagnostic_lines(diagnostics: tuple[TaskDiagnostic, ...]) -> list[str]:
    lines: list[str] = []
    for diagnostic in diagnostics:
        lines.extend(diagnostic.raw_lines)
    return lines


def _task_id_from_path(task_path: Path | None) -> str:
    if task_path is None:
        return ""
    match = _TASK_ARTIFACT_PATTERN.match(task_path.name)
    if not match:
        return ""
    return match.group(1).zfill(3)


def _batch_id_from_path(task_path: Path | None) -> str:
    if task_path is None or task_path.parent is None:
        return ""
    return task_path.parent.name


def _read_int(value: object, *, default: int = 0) -> int:
    if isinstance(value, bool):
        return int(value)
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        try:
            return int(value)
        except ValueError:
            return default
    return default


def _read_optional_int(value: object) -> int | None:
    if value is None:
        return None
    text = str(value).strip()
    if not text and not isinstance(value, int):
        return None
    return _read_int(value, default=0)


def _build_source_fingerprint(
    *,
    source_mtime_ns: int | None,
    source_size_bytes: int | None,
    source_sha256: str,
):
    if source_mtime_ns is None or source_size_bytes is None or not source_sha256:
        return None
    return SourceFingerprint(
        mtime_ns=source_mtime_ns,
        size_bytes=source_size_bytes,
        sha256=source_sha256,
    )
