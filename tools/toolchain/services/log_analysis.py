from __future__ import annotations

import re
from pathlib import Path

_ERROR_PATTERN = re.compile(
    r"(error|failed|fatal|exception|traceback|not found|undefined reference|linker command failed)",
    re.IGNORECASE,
)


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


def extract_key_error_lines_from_log(log_path: Path, limit: int = 5) -> list[str]:
    if not log_path.exists() or not log_path.is_file():
        return []
    try:
        content = log_path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return []
    return extract_key_error_lines_from_text(content, limit=limit)
