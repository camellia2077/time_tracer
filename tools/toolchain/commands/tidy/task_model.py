from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
import re

from ...services import log_parser

TASK_RECORD_VERSION = 2

_NOISE_PREFIXES = (
    "Suppressed ",
    "Use -header-filter=",
    "Found compiler error(s).",
    "Error while processing ",
)
_SNIPPET_CODE_PATTERN = re.compile(r"^\s*(\d+)\s*\|\s?(.*)$")
_SNIPPET_CARET_PATTERN = re.compile(r"^\s*\|\s?(.*)$")
_TASK_ARTIFACT_PATTERN = re.compile(r"^task_(\d+)\.(?:json|log|toon)$")


@dataclass(frozen=True, slots=True)
class TaskSummaryEntry:
    name: str
    count: int


@dataclass(frozen=True, slots=True)
class TaskSnippet:
    diagnostic_index: int
    source_line: int | None
    code: str
    caret: str
    notes: tuple[str, ...]


@dataclass(frozen=True, slots=True)
class TaskDiagnostic:
    file: str
    line: int
    col: int
    severity: str
    check: str
    message: str
    raw_lines: tuple[str, ...]
    notes: tuple[str, ...]

    def to_log_parser_dict(self) -> dict:
        return {
            "file": self.file,
            "line": self.line,
            "col": self.col,
            "severity": self.severity,
            "message": self.message,
            "check": self.check,
            "lines": list(self.raw_lines),
        }


@dataclass(frozen=True, slots=True)
class TaskSummary:
    diagnostic_count: int
    compiler_errors: bool
    files: tuple[TaskSummaryEntry, ...]
    checks: tuple[TaskSummaryEntry, ...]


@dataclass(frozen=True, slots=True)
class TaskRecord:
    version: int
    task_id: str
    batch_id: str
    source_file: str
    workspace: str
    source_scope: str | None
    checks: tuple[str, ...]
    summary: TaskSummary
    diagnostics: tuple[TaskDiagnostic, ...]
    snippets: tuple[TaskSnippet, ...]
    raw_lines: tuple[str, ...]

    def artifact_stem(self) -> str:
        return f"task_{self.task_id}"

    def diagnostic_dicts(self) -> list[dict]:
        return [diagnostic.to_log_parser_dict() for diagnostic in self.diagnostics]


@dataclass(frozen=True, slots=True)
class TaskDraft:
    source_file: str
    checks: tuple[str, ...]
    summary: TaskSummary
    diagnostics: tuple[TaskDiagnostic, ...]
    snippets: tuple[TaskSnippet, ...]
    raw_lines: tuple[str, ...]


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
    snippets = tuple(_extract_snippet(index + 1, diagnostic) for index, diagnostic in enumerate(diagnostics))
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
    workspace: str,
    source_scope: str | None,
) -> TaskRecord:
    return TaskRecord(
        version=TASK_RECORD_VERSION,
        task_id=task_id,
        batch_id=batch_id,
        source_file=draft.source_file,
        workspace=workspace,
        source_scope=source_scope,
        checks=draft.checks,
        summary=draft.summary,
        diagnostics=draft.diagnostics,
        snippets=draft.snippets,
        raw_lines=draft.raw_lines,
    )


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
            snippet = _extract_snippet(index, diagnostic)
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


def task_record_from_dict(payload: dict, *, fallback_path: Path | None = None) -> TaskRecord:
    task_id = _read_text(payload.get("task_id")) or _task_id_from_path(fallback_path)
    batch_id = _read_text(payload.get("batch_id")) or _batch_id_from_path(fallback_path)
    source_file = _read_text(payload.get("source_file"))
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
        payload.get(
            "compiler_errors",
            inferred_summary.compiler_errors,
        )
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
        source_file=source_file,
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
            _extract_snippet(index + 1, diagnostic) for index, diagnostic in enumerate(diagnostics)
        )
        if snippet is not None
    )
    return TaskRecord(
        version=TASK_RECORD_VERSION,
        task_id=_task_id_from_path(task_path),
        batch_id=_batch_id_from_path(task_path),
        source_file=source_file,
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
    lines = content.splitlines()
    section = ""
    task_id = _task_id_from_path(task_path)
    batch_id = _batch_id_from_path(task_path)
    source_file = ""
    workspace = ""
    source_scope: str | None = None
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
            elif key == "source_scope":
                source_scope = parsed or None
            continue
        if section == "summary" and ":" in value:
            key, item = value.split(":", 1)
            parsed = item.strip()
            if key == "compiler_errors":
                compiler_errors = parsed.lower() == "true"
            continue
        if section == "diagnostics":
            parts = value.split(",", 5)
            if len(parts) != 6:
                continue
            diagnostic_rows.append(
                {
                    "index": _read_int(parts[0], default=len(diagnostic_rows) + 1),
                    "line": _read_int(parts[1]),
                    "col": _read_int(parts[2]),
                    "severity": parts[3].strip(),
                    "check": parts[4].strip(),
                    "message": parts[5].strip(),
                }
            )
            continue
        if section == "snippets":
            parts = value.split(",", 3)
            if len(parts) != 4:
                continue
            snippet_rows[_read_int(parts[0], default=0)] = {
                "source_line": _read_optional_int(parts[1]),
                "code": parts[2].strip(),
                "caret": parts[3].strip(),
            }

    diagnostics_list: list[TaskDiagnostic] = []
    snippets_list: list[TaskSnippet] = []
    for item in diagnostic_rows:
        index = int(item["index"])
        snippet_item = snippet_rows.get(index, {})
        snippet = TaskSnippet(
            diagnostic_index=index,
            source_line=snippet_item.get("source_line"),
            code=str(snippet_item.get("code", "")),
            caret=str(snippet_item.get("caret", "")),
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
                    snippet_notes=(),
                    notes=(),
                ),
                notes=(),
            )
        )

    diagnostics = tuple(diagnostics_list)
    checks = tuple(_collect_checks(diagnostics))
    inferred_summary = _build_summary(diagnostics, source_file=source_file)
    summary = TaskSummary(
        diagnostic_count=len(diagnostics),
        compiler_errors=compiler_errors or inferred_summary.compiler_errors,
        files=inferred_summary.files,
        checks=inferred_summary.checks,
    )
    return TaskRecord(
        version=TASK_RECORD_VERSION,
        task_id=task_id,
        batch_id=batch_id,
        source_file=source_file,
        workspace=workspace,
        source_scope=source_scope,
        checks=checks,
        summary=summary,
        diagnostics=diagnostics,
        snippets=tuple(snippets_list),
        raw_lines=tuple(_flatten_diagnostic_lines(diagnostics)),
    )


def render_text(record: TaskRecord) -> str:
    lines: list[str] = [f"File: {record.source_file}"]

    if record.summary.checks:
        checks_text = ", ".join(f"{entry.name}({entry.count})" for entry in record.summary.checks)
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
        lines.append(
            f"snippets[{len(record.snippets)}]"
            "{diag,line,code,caret}:"
        )
        for snippet in record.snippets:
            source_line = "" if snippet.source_line is None else str(snippet.source_line)
            code = snippet.code.replace("\n", " ").replace(",", ";")
            caret = snippet.caret.replace("\n", " ").replace(",", ";")
            lines.append(
                f"  {snippet.diagnostic_index},{source_line},{code},{caret}"
            )
    return "\n".join(line for line in lines if line != "") + "\n"


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


def _extract_snippet(
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
    lines = [
        f"{file}:{line}:{col}: {severity}: {message} [{check}]",
    ]
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


def _read_dict(value: object) -> dict:
    if isinstance(value, dict):
        return value
    return {}


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
    number = _read_int(value, default=0)
    return number
