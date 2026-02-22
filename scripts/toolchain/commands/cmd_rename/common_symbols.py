import re
from pathlib import Path


def resolve_position(file_path: Path, line: int, col: int, old_name: str) -> tuple[int, int]:
    text = file_path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    if line <= 0 or line > len(lines):
        return line, col

    source_line = lines[line - 1]
    expected_col = max(1, col)
    expected_index = expected_col - 1
    if (
        old_name
        and expected_index < len(source_line)
        and source_line[expected_index : expected_index + len(old_name)] == old_name
    ):
        return line, expected_col

    if old_name:
        fallback_index = source_line.find(old_name)
        if fallback_index >= 0:
            return line, fallback_index + 1

        full_text_match = re.search(rf"\b{re.escape(old_name)}\b", text, flags=re.MULTILINE)
        if full_text_match:
            prior_text = text[: full_text_match.start()]
            fallback_line = prior_text.count("\n") + 1
            line_start = prior_text.rfind("\n")
            fallback_col = (
                full_text_match.start() + 1
                if line_start < 0
                else (full_text_match.start() - line_start)
            )
            return fallback_line, fallback_col

    return line, expected_col


def is_header_file(file_path: Path) -> bool:
    return file_path.suffix.lower() in {".h", ".hpp", ".hh", ".hxx"}


def is_risky_symbol_kind(symbol_kind: str) -> bool:
    normalized_kind = symbol_kind.strip().lower()
    return normalized_kind in {
        "function",
        "method",
        "member",
        "class member",
        "private member",
        "protected member",
    }


def should_skip_partial_header_rename(
    skip_header_single_edit: bool,
    symbol_kind: str,
    source_file: Path,
    edit_count: int,
    changed_files: list[str],
) -> bool:
    if not skip_header_single_edit:
        return False

    if edit_count > 1:
        return False

    if not is_header_file(source_file):
        return False

    if not is_risky_symbol_kind(symbol_kind):
        return False

    if not changed_files:
        return True

    for changed in changed_files:
        if not is_header_file(Path(changed)):
            return False
    return True


def count_symbol_occurrences(text: str, symbol_name: str) -> int:
    if not symbol_name:
        return 0
    return len(re.findall(rf"\b{re.escape(symbol_name)}\b", text))


def count_symbol_in_sibling_sources(
    header_path: Path,
    old_name: str,
    new_name: str,
) -> tuple[int, int]:
    old_count = 0
    new_count = 0
    for ext in (".cpp", ".cc", ".cxx"):
        sibling_path = header_path.with_suffix(ext)
        if not sibling_path.exists():
            continue
        sibling_text = sibling_path.read_text(encoding="utf-8", errors="replace")
        old_count += count_symbol_occurrences(sibling_text, old_name)
        new_count += count_symbol_occurrences(sibling_text, new_name)
    return old_count, new_count
