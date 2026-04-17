from __future__ import annotations

import json
import re
from dataclasses import dataclass
from pathlib import Path

from ...services.log_analysis import summarize_failure_text

_FAILED_TASK_PATTERN = re.compile(r"^\s*>\s*Task\s+(\S+)\s+FAILED\s*$", re.MULTILINE)
_FAILED_TEST_PATTERN = re.compile(
    r"^\s*([A-Za-z0-9_.$]+)\s*>\s*([^\r\n]+?)\s+FAILED\s*$",
    re.MULTILINE,
)
_SOURCE_LOCATION_PATTERN = re.compile(
    r"([A-Za-z0-9_./\\-]+(?:Test|Tests|Kt|Java)\.(?:kt|java)):(\d+)"
)
_TEST_TASK_HINT_PATTERN = re.compile(r":[\w-]*test[\w-]*", re.IGNORECASE)
_ANDROID_PRIMARY_ERROR_PATTERNS = (
    re.compile(r"^\s*Caused by:", re.IGNORECASE),
    re.compile(r"\bexception\b", re.IGNORECASE),
    re.compile(r"\berror:", re.IGNORECASE),
    re.compile(r"\bfailed\b", re.IGNORECASE),
)


@dataclass(frozen=True)
class AndroidFailureSummary:
    profile: str
    stage: str | None
    gradle_task: str | None
    primary_error: str | None
    key_lines: list[str]
    suspected_tests: list[str]
    source_locations: list[str]
    log_paths: list[str]


def _read_text(path: Path | None) -> str:
    if path is None or not path.exists() or not path.is_file():
        return ""
    try:
        return path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return ""


def _read_json(path: Path | None) -> dict[str, object]:
    if path is None or not path.exists() or not path.is_file():
        return {}
    try:
        return json.loads(path.read_text(encoding="utf-8", errors="replace"))
    except (OSError, json.JSONDecodeError):
        return {}


def _extract_stage(result_payload: dict[str, object]) -> str | None:
    phases = result_payload.get("verify_phases")
    if not isinstance(phases, list):
        return None
    for phase in phases:
        if isinstance(phase, dict) and str(phase.get("status", "")).lower() == "failed":
            name = str(phase.get("name", "")).strip()
            if name:
                return name
    return None


def _extract_gradle_task(build_text: str, key_lines: list[str]) -> str | None:
    tasks = _FAILED_TASK_PATTERN.findall(build_text)
    if tasks:
        return tasks[-1]
    for line in key_lines:
        match = _TEST_TASK_HINT_PATTERN.search(line)
        if match:
            return match.group(0)
    return None


def _extract_suspected_tests(text: str) -> list[str]:
    seen: set[str] = set()
    selected: list[str] = []
    for match in _FAILED_TEST_PATTERN.finditer(text):
        candidate = f"{match.group(1)} > {match.group(2).strip()}"
        if candidate in seen:
            continue
        seen.add(candidate)
        selected.append(candidate)
    return selected[:5]


def _extract_source_locations(text: str) -> list[str]:
    seen: set[str] = set()
    selected: list[str] = []
    for match in _SOURCE_LOCATION_PATTERN.finditer(text):
        candidate = f"{match.group(1)}:{match.group(2)}"
        if candidate in seen:
            continue
        seen.add(candidate)
        selected.append(candidate)
    return selected[:5]


def _pick_android_primary_error(
    *, primary_error: str | None, key_lines: list[str], build_text: str, aggregated_text: str
) -> str | None:
    candidates: list[str] = []
    for line in (build_text.splitlines() + aggregated_text.splitlines() + key_lines):
        normalized = line.strip()
        if normalized:
            candidates.append(normalized)

    for pattern in _ANDROID_PRIMARY_ERROR_PATTERNS:
        for line in candidates:
            if pattern.search(line):
                return line

    return primary_error


def build_android_failure_summary(
    *,
    profile: str,
    build_log_path: Path | None = None,
    aggregated_log_path: Path | None = None,
    full_log_path: Path | None = None,
    result_json_path: Path | None = None,
) -> AndroidFailureSummary:
    build_text = _read_text(build_log_path)
    aggregated_text = _read_text(aggregated_log_path)
    full_text = _read_text(full_log_path)
    result_payload = _read_json(result_json_path)

    analysis_chunks = [chunk for chunk in (build_text, aggregated_text, full_text) if chunk]
    analysis_text = "\n".join(analysis_chunks)
    failure = summarize_failure_text(analysis_text, limit=8)

    log_paths = [
        str(path)
        for path in (
            build_log_path,
            aggregated_log_path,
            full_log_path,
            result_json_path,
        )
        if path is not None
    ]

    suspected_tests = _extract_suspected_tests("\n".join((build_text, aggregated_text)))
    source_locations = _extract_source_locations("\n".join((build_text, aggregated_text)))

    return AndroidFailureSummary(
        profile=profile,
        stage=_extract_stage(result_payload),
        gradle_task=_extract_gradle_task(build_text, failure.key_error_lines),
        primary_error=_pick_android_primary_error(
            primary_error=failure.primary_error,
            key_lines=failure.key_error_lines,
            build_text=build_text,
            aggregated_text=aggregated_text,
        ),
        key_lines=failure.key_error_lines[:8],
        suspected_tests=suspected_tests,
        source_locations=source_locations,
        log_paths=log_paths,
    )


def render_android_failure_summary(summary: AndroidFailureSummary) -> str:
    lines = [
        "=== ANDROID CI FAILURE SUMMARY ===",
        f"profile: {summary.profile}",
        f"stage: {summary.stage or 'unknown'}",
        f"gradle_task: {summary.gradle_task or 'unknown'}",
        "",
        "primary_error:",
        summary.primary_error or "(not detected)",
        "",
        "key_lines:",
    ]

    if summary.key_lines:
        lines.extend(f"- {line}" for line in summary.key_lines)
    else:
        lines.append("- (no key lines extracted)")

    if summary.suspected_tests:
        lines.extend(["", "suspected_tests:"])
        lines.extend(f"- {item}" for item in summary.suspected_tests)

    if summary.source_locations:
        lines.extend(["", "source_locations:"])
        lines.extend(f"- {item}" for item in summary.source_locations)

    lines.extend(["", "artifacts:"])
    lines.extend(f"- {path}" for path in summary.log_paths)
    return "\n".join(lines) + "\n"
