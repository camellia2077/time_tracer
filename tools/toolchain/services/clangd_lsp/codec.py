from __future__ import annotations

import os
from pathlib import Path
from urllib.parse import unquote, urlparse


def path_to_uri(path: Path) -> str:
    return path.resolve().as_uri()


def uri_to_path(uri: str) -> Path:
    parsed = urlparse(uri)
    if parsed.scheme != "file":
        raise ValueError(f"Unsupported URI scheme in {uri}")
    path_str = unquote(parsed.path)
    if os.name == "nt" and path_str.startswith("/"):
        path_str = path_str[1:]
    return Path(path_str)


def utf16_character_to_index(text: str, utf16_character: int) -> int:
    if utf16_character <= 0:
        return 0
    units = 0
    for index, char in enumerate(text):
        if units >= utf16_character:
            return index
        units += 2 if ord(char) > 0xFFFF else 1
    return len(text)


def line_starts(text: str) -> list[int]:
    starts = [0]
    for index, char in enumerate(text):
        if char == "\n":
            starts.append(index + 1)
    return starts


def position_to_offset(text: str, line: int, character: int) -> int:
    starts = line_starts(text)
    if line < 0:
        return 0
    if line >= len(starts):
        return len(text)

    start = starts[line]
    end = starts[line + 1] if line + 1 < len(starts) else len(text)
    line_text = text[start:end]
    char_index = utf16_character_to_index(line_text, character)
    return min(start + char_index, end)
