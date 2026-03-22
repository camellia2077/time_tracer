from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

_ERROR_PATTERN = re.compile(
    r"(error|failed|fatal|exception|traceback|not found|undefined reference|linker command failed)",
    re.IGNORECASE,
)
_PRIMARY_ERROR_PATTERNS = (
    re.compile(r"\bfatal error:", re.IGNORECASE),
    re.compile(r"\berror:", re.IGNORECASE),
    re.compile(r"\bexception\b", re.IGNORECASE),
    re.compile(r"\btraceback\b", re.IGNORECASE),
    re.compile(r"\bundefined reference\b", re.IGNORECASE),
    re.compile(r"\bunresolved external symbol\b", re.IGNORECASE),
    re.compile(r"\blink(er)? command failed\b", re.IGNORECASE),
)
_CODE_EXCERPT_PATTERN = re.compile(r"^\d+\s+\|")


@dataclass(frozen=True)
class FailureSummary:
    key_error_lines: list[str]
    primary_error: str | None
    likely_fix: str | None


def extract_key_error_lines_from_text(text: str, limit: int = 5) -> list[str]:
    lines = text.splitlines()
    if not lines:
        return []

    selected: list[str] = []
    seen: set[str] = set()

    for raw_line in reversed(lines):
        line = raw_line.strip()
        if not line:
            continue
        if _ERROR_PATTERN.search(line):
            if line in seen:
                continue
            selected.append(line)
            seen.add(line)
            if len(selected) >= limit:
                break

    if not selected:
        for raw_line in reversed(lines):
            line = raw_line.strip()
            if not line:
                continue
            if line in seen:
                continue
            selected.append(line)
            seen.add(line)
            if len(selected) >= min(3, limit):
                break

    selected.reverse()
    return selected


def _pick_primary_error(key_error_lines: list[str]) -> str | None:
    filtered = [
        line
        for line in key_error_lines
        if line and not _CODE_EXCERPT_PATTERN.match(line)
    ]
    if not filtered:
        return None

    for pattern in _PRIMARY_ERROR_PATTERNS:
        for line in filtered:
            if pattern.search(line):
                return line
    return filtered[0]


def _detect_likely_fix(text: str, key_error_lines: list[str]) -> str | None:
    combined = "\n".join([text, *key_error_lines])

    if re.search(
        r"no member named '(cerr|cout|clog)' in namespace 'std'",
        combined,
        re.IGNORECASE,
    ):
        return "Add `#include <iostream>` to the failing source file before using standard stream objects."

    if re.search(r"fatal error: .* file not found", combined, re.IGNORECASE):
        return "Add the missing include/header dependency or fix the include path for the failing target."

    if re.search(
        r"(undefined reference|unresolved external symbol|linker command failed)",
        combined,
        re.IGNORECASE,
    ):
        return "Check target linkage and missing object/library sources for the failing target."

    if re.search(
        r"(module file .* not found|unable to open output file .*\.pcm)",
        combined,
        re.IGNORECASE,
    ):
        return "Clean the build directory and rebuild; if the issue persists, inspect C++ module dependency wiring and file-lock conflicts."

    if re.search(r"\btraceback\b", combined, re.IGNORECASE):
        return "Inspect the first project-local frame in the Python traceback and fix that code path before rerunning."

    if re.search(r"ninja: build stopped: subcommand failed", combined, re.IGNORECASE):
        return "Inspect the compiler or linker error lines above; the ninja stop line is only a downstream symptom."

    return None


def summarize_failure_text(text: str, limit: int = 5) -> FailureSummary:
    key_error_lines = extract_key_error_lines_from_text(text, limit=limit)
    return FailureSummary(
        key_error_lines=key_error_lines,
        primary_error=_pick_primary_error(key_error_lines),
        likely_fix=_detect_likely_fix(text, key_error_lines),
    )


def extract_key_error_lines_from_log(log_path: Path, limit: int = 5) -> list[str]:
    if not log_path.exists() or not log_path.is_file():
        return []
    try:
        content = log_path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return []
    return extract_key_error_lines_from_text(content, limit=limit)


def summarize_failure_log(log_path: Path, limit: int = 5) -> FailureSummary:
    if not log_path.exists() or not log_path.is_file():
        return FailureSummary(
            key_error_lines=[],
            primary_error=None,
            likely_fix=None,
        )
    try:
        content = log_path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return FailureSummary(
            key_error_lines=[],
            primary_error=None,
            likely_fix=None,
        )
    return summarize_failure_text(content, limit=limit)
