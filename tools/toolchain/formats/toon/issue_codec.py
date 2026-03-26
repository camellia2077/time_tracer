from __future__ import annotations

from pathlib import Path
import re

from ...commands.analyze.issue_types import AnalyzeIssue, AnalyzeIssueLocation
from .base import escape_cell, split_row

_ISSUE_ARTIFACT_PATTERN = re.compile(r"^issue_(\d+)\.(?:json|toon)$")


def render_issue(issue: AnalyzeIssue) -> str:
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
            f"  message: {escape_cell(issue.message)}",
            f"  fingerprint: {issue.fingerprint}",
            "primary{file,line,col,end_line,end_col,message}:",
            (
                "  "
                f"{issue.primary_location.file},"
                f"{issue.primary_location.line},"
                f"{issue.primary_location.col},"
                f"{issue.primary_location.end_line},"
                f"{issue.primary_location.end_col},"
                f"{escape_cell(issue.primary_location.message)}"
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
            f"{escape_cell(event.message)}"
        )
    return "\n".join(lines) + "\n"


def parse_issue(content: str, *, issue_path: Path, version: int) -> AnalyzeIssue:
    lines = content.splitlines()
    section = ""
    issue_id = _issue_id_from_path(issue_path)
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
            parts = split_row(line[2:], expected_parts=6, maxsplit=5)
            if parts is not None:
                primary = AnalyzeIssueLocation(
                    file=parts[0],
                    line=_read_int(parts[1]),
                    col=_read_int(parts[2]),
                    end_line=_read_int(parts[3]),
                    end_col=_read_int(parts[4]),
                    message=parts[5],
                )
            continue
        if section == "events" and line.startswith("  "):
            parts = split_row(line[2:], expected_parts=7, maxsplit=6)
            if parts is not None:
                events.append(
                    AnalyzeIssueLocation(
                        file=parts[1],
                        line=_read_int(parts[2]),
                        col=_read_int(parts[3]),
                        end_line=_read_int(parts[4]),
                        end_col=_read_int(parts[5]),
                        message=parts[6],
                    )
                )

    return AnalyzeIssue(
        version=version,
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


def _issue_id_from_path(issue_path: Path | None) -> str:
    if issue_path is None:
        return ""
    match = _ISSUE_ARTIFACT_PATTERN.match(issue_path.name)
    if not match:
        return ""
    return match.group(1).zfill(3)


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
