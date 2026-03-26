from __future__ import annotations

from pathlib import Path
import re

from ...formats.toon.issue_codec import parse_issue, render_issue
from .issue_types import AnalyzeIssue, AnalyzeIssueLocation

ISSUE_RECORD_VERSION = 1
_ISSUE_ARTIFACT_PATTERN = re.compile(r"^issue_(\d+)\.(?:json|toon)$")


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
    return parse_issue(content, issue_path=issue_path, version=ISSUE_RECORD_VERSION)


def render_toon(issue: AnalyzeIssue) -> str:
    return render_issue(issue)


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
