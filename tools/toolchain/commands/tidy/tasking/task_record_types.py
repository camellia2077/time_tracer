from __future__ import annotations

from dataclasses import dataclass

TASK_RECORD_VERSION = 3


@dataclass(frozen=True, slots=True)
class SourceFingerprint:
    mtime_ns: int
    size_bytes: int
    sha256: str


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
    queue_generation: int | None
    source_file: str
    source_fingerprint: SourceFingerprint | None
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
