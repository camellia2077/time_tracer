from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class AnalyzeIssueLocation:
    file: str
    line: int
    col: int
    end_line: int
    end_col: int
    message: str

    def to_dict(self) -> dict:
        return {
            "file": self.file,
            "line": self.line,
            "col": self.col,
            "end_line": self.end_line,
            "end_col": self.end_col,
            "message": self.message,
        }


@dataclass(frozen=True, slots=True)
class AnalyzeIssue:
    version: int
    issue_id: str
    batch_id: str
    workspace: str
    source_scope: str | None
    source_file: str
    rule_id: str
    category: str
    severity: str
    message: str
    primary_location: AnalyzeIssueLocation
    events: tuple[AnalyzeIssueLocation, ...]
    fingerprint: str
    raw_report: dict

    def to_dict(self) -> dict:
        return {
            "version": self.version,
            "issue_id": self.issue_id,
            "batch_id": self.batch_id,
            "workspace": self.workspace,
            "source_scope": self.source_scope,
            "source_file": self.source_file,
            "rule_id": self.rule_id,
            "category": self.category,
            "severity": self.severity,
            "message": self.message,
            "primary_location": self.primary_location.to_dict(),
            "events": [event.to_dict() for event in self.events],
            "fingerprint": self.fingerprint,
            "raw_report": dict(self.raw_report),
        }
