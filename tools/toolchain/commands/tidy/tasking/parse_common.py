from __future__ import annotations

from collections import defaultdict
from pathlib import Path
import re

from ....services import log_parser
from .task_record_types import (
    SourceFingerprint,
    TaskDiagnostic,
    TaskSnippet,
    TaskSummary,
    TaskSummaryEntry,
)

_NOISE_PREFIXES = (
    "Suppressed ",
    "Use -header-filter=",
    "Found compiler error(s).",
    "Error while processing ",
)
_SNIPPET_CODE_PATTERN = re.compile(r"^\s*(\d+)\s*\|\s?(.*)$")
_SNIPPET_CARET_PATTERN = re.compile(r"^\s*\|\s?(.*)$")
_TASK_ARTIFACT_PATTERN = re.compile(r"^task_(\d+)\.(?:json|log|toon)$")


def task_id_from_artifact_name(file_name: str) -> str | None:
    match = _TASK_ARTIFACT_PATTERN.match(file_name)
    if not match:
        return None
    return match.group(1).zfill(3)


def build_diagnostic(item: dict) -> TaskDiagnostic:
    raw_lines = tuple(read_str_list(item.get("lines")))
    notes = tuple(extract_filtered_notes(raw_lines[1:]))
    return TaskDiagnostic(
        file=read_text(item.get("file")),
        line=read_int(item.get("line")),
        col=read_int(item.get("col")),
        severity=read_text(item.get("severity")),
        check=read_text(item.get("check")),
        message=read_text(item.get("message")),
        raw_lines=raw_lines,
        notes=notes,
    )


def build_summary(
    diagnostics: tuple[TaskDiagnostic, ...],
    *,
    source_file: str,
) -> TaskSummary:
    file_counts: defaultdict[str, int] = defaultdict(int)
    check_counts: defaultdict[str, int] = defaultdict(int)
    compiler_errors = False

    if source_file:
        file_counts[source_file] += 0

    for diagnostic in diagnostics:
        if diagnostic.file:
            file_counts[diagnostic.file] += 1
        if diagnostic.check:
            check_counts[diagnostic.check] += 1
        if diagnostic.severity == "error" or diagnostic.check == "clang-diagnostic-error":
            compiler_errors = True

    return TaskSummary(
        diagnostic_count=len(diagnostics),
        compiler_errors=compiler_errors,
        files=tuple(
            TaskSummaryEntry(name=name, count=count)
            for name, count in sorted(file_counts.items(), key=lambda item: (-item[1], item[0]))
        ),
        checks=tuple(
            TaskSummaryEntry(name=name, count=count)
            for name, count in sorted(check_counts.items(), key=lambda item: (-item[1], item[0]))
        ),
    )


def flatten_diagnostic_lines(diagnostics: tuple[TaskDiagnostic, ...]) -> list[str]:
    lines: list[str] = []
    for diagnostic in diagnostics:
        lines.extend(diagnostic.raw_lines)
    return lines


def collect_checks(diagnostics: tuple[TaskDiagnostic, ...]) -> list[str]:
    checks: list[str] = []
    for diagnostic in diagnostics:
        if diagnostic.check and diagnostic.check not in checks:
            checks.append(diagnostic.check)
    return checks


def extract_filtered_notes(lines: tuple[str, ...] | list[str]) -> list[str]:
    filtered: list[str] = []
    for raw_line in lines:
        clean_line = log_parser.ANSI_ESCAPE_PATTERN.sub("", raw_line).rstrip()
        if not clean_line.strip():
            continue
        if _SNIPPET_CODE_PATTERN.match(clean_line):
            continue
        if _SNIPPET_CARET_PATTERN.match(clean_line):
            continue
        if is_noise_line(clean_line):
            continue
        filtered.append(clean_line.strip())
    return filtered


def compose_raw_lines(
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
    snippet_notes: tuple[str, ...],
    notes: tuple[str, ...],
) -> tuple[str, ...]:
    lines = [f"{file}:{line}:{col}: {severity}: {message} [{check}]"]
    if code:
        if source_line is None:
            lines.append(f"      | {code}")
        else:
            lines.append(f"    {source_line} | {code}")
    if caret:
        lines.append(f"      | {caret}")
    lines.extend(snippet_notes)
    lines.extend(notes)
    return tuple(lines)


def is_noise_line(text: str) -> bool:
    normalized = text.strip()
    if not normalized:
        return False
    return normalized.startswith(_NOISE_PREFIXES)


def task_id_from_path(task_path: Path | None) -> str:
    if task_path is None:
        return ""
    task_id = task_id_from_artifact_name(task_path.name)
    return task_id or ""


def batch_id_from_path(task_path: Path | None) -> str:
    if task_path is None or task_path.parent is None:
        return ""
    return task_path.parent.name


def read_dict_list(value: object) -> list[dict]:
    if not isinstance(value, list):
        return []
    return [item for item in value if isinstance(item, dict)]


def read_str_list(value: object) -> list[str]:
    if not isinstance(value, list):
        return []
    result: list[str] = []
    for item in value:
        if isinstance(item, str):
            result.append(item)
    return result


def read_text(value: object) -> str:
    if isinstance(value, str):
        return value.strip()
    return ""


def read_optional_text(value: object) -> str | None:
    text = read_text(value)
    return text or None


def read_int(value: object, *, default: int = 0) -> int:
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


def read_optional_int(value: object) -> int | None:
    if value is None:
        return None
    text = read_text(value)
    if not text and not isinstance(value, int):
        return None
    return read_int(value, default=0)


def read_source_fingerprint(value: object) -> SourceFingerprint | None:
    if not isinstance(value, dict):
        return None
    sha256 = read_text(value.get("sha256"))
    if not sha256:
        return None
    return SourceFingerprint(
        mtime_ns=read_int(value.get("mtime_ns")),
        size_bytes=read_int(value.get("size_bytes")),
        sha256=sha256,
    )


def extract_snippet(
    diagnostic_index: int,
    diagnostic: TaskDiagnostic,
) -> TaskSnippet | None:
    source_line_number: int | None = None
    code = ""
    caret = ""
    notes: list[str] = []

    for raw_line in diagnostic.raw_lines[1:]:
        clean_line = log_parser.ANSI_ESCAPE_PATTERN.sub("", raw_line).rstrip()
        if not clean_line.strip():
            continue
        code_match = _SNIPPET_CODE_PATTERN.match(clean_line)
        if code_match and not code:
            source_line_number = read_optional_int(code_match.group(1))
            code = code_match.group(2).rstrip()
            continue
        caret_match = _SNIPPET_CARET_PATTERN.match(clean_line)
        if caret_match and not caret:
            caret_candidate = caret_match.group(1).rstrip()
            if caret_candidate:
                caret = caret_candidate
                continue
        if is_noise_line(clean_line):
            continue
        notes.append(clean_line.strip())

    if not code and not caret and not notes:
        return None
    return TaskSnippet(
        diagnostic_index=diagnostic_index,
        source_line=source_line_number,
        code=code,
        caret=caret,
        notes=tuple(notes),
    )
