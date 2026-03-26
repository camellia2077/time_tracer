from __future__ import annotations

import re

from ..models import WorkspaceTextEdit

_ANGLE_INCLUDE_PATTERN = re.compile(r"^\s*#include\s+<(?P<header>[^>]+)>\s*$")
_QUOTE_INCLUDE_PATTERN = re.compile(r'^\s*#include\s+"[^"]+"\s*$')


def ensure_standard_include(
    text: str,
    *,
    file_path: str,
    header: str,
) -> WorkspaceTextEdit | None:
    include_line = f"#include <{header}>"
    if include_line in text.splitlines():
        return None
    insertion = find_standard_include_insertion_offset(text, header)
    return WorkspaceTextEdit(
        file_path=file_path,
        start_offset=insertion,
        end_offset=insertion,
        new_text=include_line + "\n",
    )


def find_standard_include_insertion_offset(text: str, header: str) -> int:
    lines = text.splitlines(keepends=True)
    angle_headers: list[tuple[int, int, str]] = []
    quote_indexes: list[int] = []
    offset = 0
    for index, line in enumerate(lines):
        stripped = line.rstrip("\r\n")
        angle_match = _ANGLE_INCLUDE_PATTERN.match(stripped)
        if angle_match:
            angle_headers.append((offset, offset + len(line), angle_match.group("header").strip()))
        elif _QUOTE_INCLUDE_PATTERN.match(stripped):
            quote_indexes.append(offset)
        offset += len(line)

    if angle_headers:
        for line_offset, _line_end, current_header in angle_headers:
            if current_header > header:
                return line_offset
        _last_offset, last_end, _last_header = angle_headers[-1]
        return last_end
    if quote_indexes:
        return quote_indexes[0]

    offset = 0
    for line in lines:
        stripped = line.strip()
        if not stripped or stripped.startswith("//"):
            offset += len(line)
            continue
        break
    return offset
