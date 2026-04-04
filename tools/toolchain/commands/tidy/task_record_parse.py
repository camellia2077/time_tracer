from __future__ import annotations

from collections import defaultdict
from pathlib import Path
import re

from ...formats.toon.task_codec import parse_task_record
from .task_fingerprint import compute_source_fingerprint
from ...services import log_parser
from .task_record_types import (
    TASK_RECORD_VERSION,
    SourceFingerprint,
    TaskDiagnostic,
    TaskDraft,
    TaskRecord,
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
    diagnostics = tuple(_build_diagnostic(item) for item in diagnostics_raw)
    checks = tuple(_collect_checks(diagnostics))
    summary = _build_summary(diagnostics, source_file=source_file)
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
        raw_lines=tuple(raw_lines or _flatten_diagnostic_lines(diagnostics)),
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


def task_record_from_dict(payload: dict, *, fallback_path: Path | None = None) -> TaskRecord:
    task_id = _read_text(payload.get("task_id")) or _task_id_from_path(fallback_path)
    batch_id = (
        _read_text(payload.get("queue_batch_id"))
        or _read_text(payload.get("batch_id"))
        or _batch_id_from_path(fallback_path)
    )
    queue_generation = _read_optional_int(payload.get("queue_generation"))
    source_file = _read_text(payload.get("source_file"))
    source_fingerprint = _read_source_fingerprint(payload.get("source_fingerprint"))
    diagnostics_payload = _read_dict_list(payload.get("diagnostics"))

    diagnostics_list: list[TaskDiagnostic] = []
    snippets_list: list[TaskSnippet] = []
    for index, item in enumerate(diagnostics_payload, 1):
        diag_file = _read_text(item.get("file")) or source_file
        line = _read_int(item.get("line"))
        col = _read_int(item.get("col"))
        severity = _read_text(item.get("severity"))
        check = _read_text(item.get("check"))
        message = _read_text(item.get("message"))
        notes = tuple(_read_str_list(item.get("notes")))
        snippet_notes = tuple(_read_str_list(item.get("snippet_notes")))
        source_line = _read_optional_int(item.get("source_line"))
        code = _read_text(item.get("code"))
        caret = _read_text(item.get("caret"))
        raw_lines_payload = _read_str_list(item.get("raw_lines"))
        if raw_lines_payload:
            raw_lines = tuple(raw_lines_payload)
        else:
            raw_lines = _compose_raw_lines(
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

    checks = tuple(_read_str_list(payload.get("checks")))
    if not checks:
        checks = tuple(_collect_checks(diagnostics))
    inferred_summary = _build_summary(diagnostics, source_file=source_file)
    compiler_errors = bool(
        payload.get("compiler_errors", inferred_summary.compiler_errors)
    )
    summary = TaskSummary(
        diagnostic_count=len(diagnostics),
        compiler_errors=compiler_errors,
        files=inferred_summary.files,
        checks=inferred_summary.checks,
    )

    return TaskRecord(
        version=_read_int(payload.get("version"), default=TASK_RECORD_VERSION),
        task_id=task_id,
        batch_id=batch_id,
        queue_generation=queue_generation,
        source_file=source_file,
        source_fingerprint=source_fingerprint,
        workspace=_read_text(payload.get("workspace")),
        source_scope=_read_optional_text(payload.get("source_scope")),
        checks=checks,
        summary=summary,
        diagnostics=diagnostics,
        snippets=snippets,
        raw_lines=tuple(_flatten_diagnostic_lines(diagnostics)),
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
    diagnostics = tuple(_build_diagnostic(item) for item in diagnostics_raw)
    if not source_file and diagnostics:
        source_file = diagnostics[0].file
    checks = tuple(_collect_checks(diagnostics))
    summary = _build_summary(diagnostics, source_file=source_file)
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
        task_id=_task_id_from_path(task_path),
        batch_id=_batch_id_from_path(task_path),
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
            source_line_number = _read_optional_int(code_match.group(1))
            code = code_match.group(2).rstrip()
            continue
        caret_match = _SNIPPET_CARET_PATTERN.match(clean_line)
        if caret_match and not caret:
            caret_candidate = caret_match.group(1).rstrip()
            if caret_candidate:
                caret = caret_candidate
                continue
        if _is_noise_line(clean_line):
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


def _build_diagnostic(item: dict) -> TaskDiagnostic:
    raw_lines = tuple(_read_str_list(item.get("lines")))
    notes = tuple(_extract_filtered_notes(raw_lines[1:]))
    return TaskDiagnostic(
        file=_read_text(item.get("file")),
        line=_read_int(item.get("line")),
        col=_read_int(item.get("col")),
        severity=_read_text(item.get("severity")),
        check=_read_text(item.get("check")),
        message=_read_text(item.get("message")),
        raw_lines=raw_lines,
        notes=notes,
    )


def _build_summary(
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


def _flatten_diagnostic_lines(diagnostics: tuple[TaskDiagnostic, ...]) -> list[str]:
    lines: list[str] = []
    for diagnostic in diagnostics:
        lines.extend(diagnostic.raw_lines)
    return lines


def _collect_checks(diagnostics: tuple[TaskDiagnostic, ...]) -> list[str]:
    checks: list[str] = []
    for diagnostic in diagnostics:
        if diagnostic.check and diagnostic.check not in checks:
            checks.append(diagnostic.check)
    return checks


def _extract_filtered_notes(lines: tuple[str, ...] | list[str]) -> list[str]:
    filtered: list[str] = []
    for raw_line in lines:
        clean_line = log_parser.ANSI_ESCAPE_PATTERN.sub("", raw_line).rstrip()
        if not clean_line.strip():
            continue
        if _SNIPPET_CODE_PATTERN.match(clean_line):
            continue
        if _SNIPPET_CARET_PATTERN.match(clean_line):
            continue
        if _is_noise_line(clean_line):
            continue
        filtered.append(clean_line.strip())
    return filtered


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


def _is_noise_line(text: str) -> bool:
    normalized = text.strip()
    if not normalized:
        return False
    return normalized.startswith(_NOISE_PREFIXES)


def _task_id_from_path(task_path: Path | None) -> str:
    if task_path is None:
        return ""
    task_id = task_id_from_artifact_name(task_path.name)
    return task_id or ""


def _batch_id_from_path(task_path: Path | None) -> str:
    if task_path is None or task_path.parent is None:
        return ""
    return task_path.parent.name


def _read_dict_list(value: object) -> list[dict]:
    if not isinstance(value, list):
        return []
    return [item for item in value if isinstance(item, dict)]


def _read_str_list(value: object) -> list[str]:
    if not isinstance(value, list):
        return []
    result: list[str] = []
    for item in value:
        if isinstance(item, str):
            result.append(item)
    return result


def _read_text(value: object) -> str:
    if isinstance(value, str):
        return value.strip()
    return ""


def _read_optional_text(value: object) -> str | None:
    text = _read_text(value)
    return text or None


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
    text = _read_text(value)
    if not text and not isinstance(value, int):
        return None
    return _read_int(value, default=0)


def _read_source_fingerprint(value: object) -> SourceFingerprint | None:
    if not isinstance(value, dict):
        return None
    sha256 = _read_text(value.get("sha256"))
    if not sha256:
        return None
    return SourceFingerprint(
        mtime_ns=_read_int(value.get("mtime_ns")),
        size_bytes=_read_int(value.get("size_bytes")),
        sha256=sha256,
    )
