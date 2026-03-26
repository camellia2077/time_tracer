from __future__ import annotations

import difflib
import re
from collections import defaultdict
from pathlib import Path

_READ_FIELD_PATTERN = re.compile(
    r"^\s*const\s+(?:auto|json|bool|int|double|std::[A-Za-z_:<>]+)\s+"
    r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*=\s*TryRead[A-Za-z0-9_]+Field\(",
    flags=re.MULTILINE,
)
_HAS_ERROR_PATTERN_TEMPLATE = r"\b{var}\.HasError\(\)"
_HAS_VALUE_PATTERN_TEMPLATE = r"\b{var}\.value\.has_value\(\)"
_VALUE_OR_PATTERN_TEMPLATE = r"\b{var}\.value\.value_or\("
_DEREF_VALUE_PATTERN_TEMPLATE = r"\*\s*{var}\.value\b"
_MAGIC_IF_PATTERN = re.compile(
    r"if\s*\(\s*(?P<var>[A-Za-z_][A-Za-z0-9_]*)\s*==\s*(?P<num>\d+)\s*\)"
)


def load_text_lines(file_path: Path) -> tuple[str, list[str]] | None:
    try:
        text = file_path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None
    return text, text.splitlines()


def resolve_line_range(text: str, line_number: int) -> tuple[int, int] | None:
    if line_number <= 0:
        return None
    lines = text.splitlines(keepends=True)
    if line_number > len(lines):
        return None
    start = sum(len(line) for line in lines[: line_number - 1])
    end = start + len(lines[line_number - 1].rstrip("\r\n"))
    return start, end


def line_text(file_path: Path, line_number: int) -> str:
    loaded = load_text_lines(file_path)
    if loaded is None:
        return ""
    _, lines = loaded
    if line_number <= 0 or line_number > len(lines):
        return ""
    return lines[line_number - 1]


def build_diff(file_path: Path, before_text: str, after_text: str) -> str:
    if before_text == after_text:
        return ""
    diff_lines = difflib.unified_diff(
        before_text.splitlines(),
        after_text.splitlines(),
        fromfile=str(file_path),
        tofile=str(file_path),
        lineterm="",
    )
    return "\n".join(diff_lines)


def strip_string_literals(text: str) -> str:
    return re.sub(r'"(?:\\.|[^"])*"', '""', text)


def has_identifier(text: str, name: str) -> bool:
    if not name:
        return False
    return bool(re.search(rf"\b{re.escape(name)}\b", strip_string_literals(text)))


def select_literal_match(
    source_line: str,
    *,
    literal: str,
    source_index: int,
) -> tuple[int, int] | None:
    matches: list[tuple[int, int]] = []
    search_from = 0
    while True:
        start = source_line.find(literal, search_from)
        if start < 0:
            break
        matches.append((start, start + len(literal)))
        search_from = start + 1
    if not matches:
        return None
    for start, end in matches:
        if start <= source_index < end:
            return start, end
    return min(
        matches,
        key=lambda item: (
            min(abs(source_index - item[0]), abs(source_index - (item[1] - 1))),
            abs(source_index - item[0]),
            item[0],
        ),
    )


def detect_task_refactors(parsed) -> list[dict]:
    source_file = Path(parsed.source_file) if parsed.source_file else None
    if source_file is None or not source_file.exists():
        return []

    content = source_file.read_text(encoding="utf-8", errors="replace")
    suggestions: list[dict] = []

    decode_vars = [match.group("name") for match in _READ_FIELD_PATTERN.finditer(content)]
    decode_ready_vars: list[str] = []
    for var_name in decode_vars:
        has_error = re.search(_HAS_ERROR_PATTERN_TEMPLATE.format(var=re.escape(var_name)), content)
        has_value = re.search(_HAS_VALUE_PATTERN_TEMPLATE.format(var=re.escape(var_name)), content)
        has_value_or = re.search(_VALUE_OR_PATTERN_TEMPLATE.format(var=re.escape(var_name)), content)
        has_deref_value = re.search(_DEREF_VALUE_PATTERN_TEMPLATE.format(var=re.escape(var_name)), content)
        if has_error and (has_value or has_value_or or has_deref_value):
            decode_ready_vars.append(var_name)
    if len(decode_ready_vars) >= 3:
        suggestions.append(
            {
                "kind": "decode_helper",
                "priority": "medium",
                "summary": "重复的 TryRead/HasError/value.has_value 骨架适合抽成 decode helper。",
                "evidence": decode_ready_vars[:8],
            }
        )

    magic_hits: dict[str, list[str]] = defaultdict(list)
    for match in _MAGIC_IF_PATTERN.finditer(content):
        magic_hits[match.group("var")].append(match.group("num"))
    for variable_name, values in sorted(magic_hits.items()):
        unique_values = sorted(set(values), key=int)
        if len(unique_values) < 3:
            continue
        suggestions.append(
            {
                "kind": "protocol_constants",
                "priority": "medium",
                "summary": "链式数值分支看起来是协议码/枚举映射，适合提取命名常量或 enum。",
                "evidence": {"variable": variable_name, "values": unique_values},
            }
        )

    return suggestions
