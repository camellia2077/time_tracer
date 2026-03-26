from __future__ import annotations

import re
from pathlib import Path

_QUOTED_INCLUDE_PATTERN = re.compile(r'^\s*#include\s+"(?P<path>[^"]+)"\s*$', re.MULTILINE)
_DTO_QUALIFIED_SYMBOL_PATTERN = re.compile(
    r"\btracer_core::core::dto::(?P<name>[A-Za-z_][A-Za-z0-9_]*)\b"
)
_DTO_DECL_PATTERN = re.compile(
    r"\b(?:struct|class|enum(?:\s+class)?)\s+(?P<decl_name>[A-Za-z_][A-Za-z0-9_]*)\b"
    r"|\busing\s+(?P<using_name>[A-Za-z_][A-Za-z0-9_]*)\s*="
)
_DTO_NAMESPACE_DECL = "namespace tracer_core::core::dto"
_DTO_USING_NAMESPACE_LINE = "using namespace tracer_core::core::dto;"
_SUPPORT_FILE_ALLOWED_SEGMENTS = ("/application/", "/libs/")
_MAX_INCLUDE_SCAN = 48
_MAX_INCLUDE_DEPTH = 4


def build_dto_using_declarations(source_file: Path, source_lines: list[str]) -> list[str]:
    support_files = _collect_support_files(source_file)
    dto_symbols = _collect_dto_symbols(support_files)
    if not dto_symbols:
        return []

    existing_declarations = {
        line.strip()
        for line in source_lines
        if line.strip().startswith("using tracer_core::core::dto::")
    }
    ordered_symbols = _ordered_used_dto_symbols(source_lines, dto_symbols)
    return [
        f"using tracer_core::core::dto::{symbol};"
        for symbol in ordered_symbols
        if f"using tracer_core::core::dto::{symbol};" not in existing_declarations
    ]


def resolve_using_namespace_line(source_lines: list[str]) -> int | None:
    matches = [
        index
        for index, line in enumerate(source_lines)
        if line.strip() == _DTO_USING_NAMESPACE_LINE
    ]
    if not matches:
        return None
    return matches[0]


def _collect_support_files(source_file: Path) -> list[Path]:
    pending: list[tuple[Path, int]] = [(source_file.resolve(), 0)]
    collected: list[Path] = []
    seen: set[Path] = set()

    while pending and len(collected) < _MAX_INCLUDE_SCAN:
        current_path, depth = pending.pop(0)
        if current_path in seen or not current_path.exists():
            continue
        seen.add(current_path)
        collected.append(current_path)
        if depth >= _MAX_INCLUDE_DEPTH:
            continue
        content = current_path.read_text(encoding="utf-8", errors="replace")
        for include_path in _QUOTED_INCLUDE_PATTERN.findall(content):
            resolved = _resolve_local_include(current_path, include_path)
            if resolved is None:
                continue
            if not _support_file_allowed(resolved):
                continue
            pending.append((resolved, depth + 1))
    return collected


def _resolve_local_include(current_path: Path, include_path: str) -> Path | None:
    include_candidate = Path(include_path)
    search_roots: list[Path] = [current_path.parent]
    resolved_current = current_path.resolve()
    for parent in resolved_current.parents:
        if parent.name.lower() == "src":
            search_roots.append(parent)

    seen_roots: set[Path] = set()
    for root in search_roots:
        resolved_root = root.resolve()
        if resolved_root in seen_roots:
            continue
        seen_roots.add(resolved_root)
        candidate = (resolved_root / include_candidate).resolve()
        if candidate.exists():
            return candidate
    return None


def _support_file_allowed(path: Path) -> bool:
    normalized = str(path).replace("\\", "/").lower()
    return any(segment in normalized for segment in _SUPPORT_FILE_ALLOWED_SEGMENTS)


def _collect_dto_symbols(paths: list[Path]) -> set[str]:
    symbols: set[str] = set()
    for path in paths:
        content = path.read_text(encoding="utf-8", errors="replace")
        symbols.update(
            match.group("name")
            for match in _DTO_QUALIFIED_SYMBOL_PATTERN.finditer(content)
            if match.group("name")
        )
        symbols.update(_dto_declared_symbols(content))
    return {
        symbol
        for symbol in symbols
        if symbol and symbol[:1].isupper()
    }


def _dto_declared_symbols(content: str) -> set[str]:
    body = _extract_namespace_body(content, _DTO_NAMESPACE_DECL)
    if not body:
        return set()
    symbols: set[str] = set()
    for match in _DTO_DECL_PATTERN.finditer(body):
        name = match.group("decl_name") or match.group("using_name") or ""
        if name:
            symbols.add(name)
    return symbols


def _extract_namespace_body(content: str, namespace_decl: str) -> str:
    namespace_index = content.find(namespace_decl)
    if namespace_index < 0:
        return ""
    brace_index = content.find("{", namespace_index)
    if brace_index < 0:
        return ""

    depth = 1
    cursor = brace_index + 1
    while cursor < len(content):
        character = content[cursor]
        if character == "{":
            depth += 1
        elif character == "}":
            depth -= 1
            if depth == 0:
                return content[brace_index + 1 : cursor]
        cursor += 1
    return ""


def _ordered_used_dto_symbols(source_lines: list[str], dto_symbols: set[str]) -> list[str]:
    body_lines: list[str] = []
    for line in source_lines:
        stripped = line.strip()
        if stripped.startswith("#include"):
            continue
        if stripped == _DTO_USING_NAMESPACE_LINE:
            continue
        body_lines.append(re.sub(r"//.*$", "", line))
    body = "\n".join(body_lines)

    matches: list[tuple[int, str]] = []
    for symbol in dto_symbols:
        match = re.search(rf"\b{re.escape(symbol)}\b", body)
        if match is None:
            continue
        matches.append((match.start(), symbol))
    matches.sort(key=lambda item: (item[0], item[1]))
    return [symbol for _, symbol in matches]
