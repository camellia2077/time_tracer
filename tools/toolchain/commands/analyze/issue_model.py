from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re

ISSUE_RECORD_VERSION = 1
_ISSUE_ARTIFACT_PATTERN = re.compile(r"^issue_(\d+)\.(?:json|toon)$")


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


def issue_id_from_artifact_name(file_name: str) -> str | None:
    match = _ISSUE_ARTIFACT_PATTERN.match(file_name)
    if not match:
        return None
    return match.group(1).zfill(3)


def issue_from_dict(payload: dict) -> AnalyzeIssue:
    primary_payload = _read_dict(payload.get("primary_location"))
    events_payload = _read_dict_list(payload.get("events"))
    return AnalyzeIssue(
        version=_read_int(payload.get("version"), default=ISSUE_RECORD_VERSION),
        issue_id=_read_text(payload.get("issue_id")),
        batch_id=_read_text(payload.get("batch_id")),
        workspace=_read_text(payload.get("workspace")),
        source_scope=_read_optional_text(payload.get("source_scope")),
        source_file=_read_text(payload.get("source_file")),
        rule_id=_read_text(payload.get("rule_id")),
        category=_read_text(payload.get("category")),
        severity=_read_text(payload.get("severity")),
        message=_read_text(payload.get("message")),
        primary_location=_location_from_payload(primary_payload),
        events=tuple(_location_from_payload(item) for item in events_payload),
        fingerprint=_read_text(payload.get("fingerprint")),
        raw_report=_read_dict(payload.get("raw_report")),
    )


def toon_issue_from_text(content: str, *, issue_path: Path) -> AnalyzeIssue:
    lines = content.splitlines()
    section = ""
    issue_id = issue_id_from_artifact_name(issue_path.name) or ""
    batch_id = issue_path.parent.name if issue_path.parent else ""
    workspace = ""
    source_scope: str | None = None
    source_file = ""
    rule_id = ""
    category = ""
    severity = ""
    message = ""
    fingerprint = ""
    primary = AnalyzeIssueLocation("", 0, 0, 0, 0, "")
    events: list[AnalyzeIssueLocation] = []

    for raw_line in lines:
        line = raw_line.rstrip()
        stripped = line.strip()
        if not stripped:
            continue
        if stripped == "issue:":
            section = "issue"
            continue
        if stripped == "summary:":
            section = "summary"
            continue
        if stripped.startswith("primary{"):
            section = "primary"
            continue
        if stripped.startswith("events["):
            section = "events"
            continue
        if section in {"issue", "summary"} and line.startswith("  ") and ":" in line:
            key, value = line[2:].split(":", 1)
            parsed = value.strip()
            if section == "issue":
                if key == "id":
                    issue_id = parsed or issue_id
                elif key == "batch":
                    batch_id = parsed or batch_id
                elif key == "source":
                    source_file = parsed
                elif key == "workspace":
                    workspace = "" if parsed == "<unset>" else parsed
                elif key == "source_scope":
                    source_scope = parsed or None
            else:
                if key == "rule":
                    rule_id = parsed
                elif key == "category":
                    category = parsed
                elif key == "severity":
                    severity = parsed
                elif key == "message":
                    message = parsed
                elif key == "fingerprint":
                    fingerprint = parsed
            continue
        if section == "primary" and line.startswith("  "):
            parts = line[2:].split(",", 5)
            if len(parts) == 6:
                primary = AnalyzeIssueLocation(
                    file=parts[0].strip(),
                    line=_read_int(parts[1]),
                    col=_read_int(parts[2]),
                    end_line=_read_int(parts[3]),
                    end_col=_read_int(parts[4]),
                    message=parts[5].strip(),
                )
            continue
        if section == "events" and line.startswith("  "):
            parts = line[2:].split(",", 6)
            if len(parts) == 7:
                events.append(
                    AnalyzeIssueLocation(
                        file=parts[1].strip(),
                        line=_read_int(parts[2]),
                        col=_read_int(parts[3]),
                        end_line=_read_int(parts[4]),
                        end_col=_read_int(parts[5]),
                        message=parts[6].strip(),
                    )
                )

    return AnalyzeIssue(
        version=ISSUE_RECORD_VERSION,
        issue_id=issue_id,
        batch_id=batch_id,
        workspace=workspace,
        source_scope=source_scope,
        source_file=source_file,
        rule_id=rule_id,
        category=category,
        severity=severity,
        message=message,
        primary_location=primary,
        events=tuple(events),
        fingerprint=fingerprint,
        raw_report={},
    )


def render_toon(issue: AnalyzeIssue) -> str:
    lines = [
        "issue:",
        f"  id: {issue.issue_id}",
        f"  batch: {issue.batch_id}",
        f"  source: {issue.source_file}",
        f"  workspace: {issue.workspace or '<unset>'}",
    ]
    if issue.source_scope:
        lines.append(f"  source_scope: {issue.source_scope}")
    lines.extend(
        [
            "summary:",
            f"  rule: {issue.rule_id}",
            f"  category: {issue.category}",
            f"  severity: {issue.severity}",
            f"  message: {_escape_toon_text(issue.message)}",
            f"  fingerprint: {issue.fingerprint}",
            "primary{file,line,col,end_line,end_col,message}:",
            (
                "  "
                f"{issue.primary_location.file},"
                f"{issue.primary_location.line},"
                f"{issue.primary_location.col},"
                f"{issue.primary_location.end_line},"
                f"{issue.primary_location.end_col},"
                f"{_escape_toon_text(issue.primary_location.message)}"
            ),
        ]
    )
    lines.append(
        f"events[{len(issue.events)}]"
        "{index,file,line,col,end_line,end_col,message}:"
    )
    for index, event in enumerate(issue.events, start=1):
        lines.append(
            "  "
            f"{index},"
            f"{event.file},"
            f"{event.line},"
            f"{event.col},"
            f"{event.end_line},"
            f"{event.end_col},"
            f"{_escape_toon_text(event.message)}"
        )
    return "\n".join(lines) + "\n"


def _escape_toon_text(text: str) -> str:
    return str(text or "").replace("\n", " ").replace(",", ";").strip()


def _location_from_payload(payload: dict) -> AnalyzeIssueLocation:
    return AnalyzeIssueLocation(
        file=_read_text(payload.get("file")),
        line=_read_int(payload.get("line")),
        col=_read_int(payload.get("col")),
        end_line=_read_int(payload.get("end_line")),
        end_col=_read_int(payload.get("end_col")),
        message=_read_text(payload.get("message")),
    )


def _read_dict(value: object) -> dict:
    if isinstance(value, dict):
        return value
    return {}


def _read_dict_list(value: object) -> list[dict]:
    if not isinstance(value, list):
        return []
    return [item for item in value if isinstance(item, dict)]


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
