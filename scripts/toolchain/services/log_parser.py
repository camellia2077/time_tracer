import re
from collections import defaultdict

# Patterns migrated from legacy log_utils
# Ninja progress format: [N/M] Task description...
TASK_START_PATTERN = re.compile(r"^\[(\d+)/(\d+)\]\s+(.+)$")
ANSI_ESCAPE_PATTERN = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")
DIAGNOSTIC_LINE_PATTERN = re.compile(
    r"^([A-Za-z]:)?([^:]+):(\d+):(\d+):\s+(warning|error):\s+(.+)$"
)
CHECK_NAME_EXTRACTOR = re.compile(r"(.+)\s+\[([^\]]+)\]$")
INVALID_CASE_STYLE_PATTERN = re.compile(r"invalid case style for ([^']+) '([^']+)'")
SUGGESTED_NAME_PATTERN = re.compile(r"^\s*\|\s+([A-Za-z_][A-Za-z0-9_]*)\s*$")


def _strip_ansi(text: str) -> str:
    return ANSI_ESCAPE_PATTERN.sub("", text)


def extract_diagnostics(lines: list[str]) -> list[dict]:
    diagnostics = []
    current_diag = None

    for line in lines:
        stripped = _strip_ansi(line).strip()
        match = DIAGNOSTIC_LINE_PATTERN.match(stripped)
        if match:
            # New top-level diagnostic (warning/error)
            drive = match.group(1) or ""
            file_path = drive + match.group(2)
            severity = match.group(5)
            full_msg = match.group(6)

            # Try to extract check name from the end of the message
            check_match = CHECK_NAME_EXTRACTOR.match(full_msg)
            if check_match:
                message = check_match.group(1)
                check_name = check_match.group(2)
            else:
                message = full_msg
                check_name = f"clang-diagnostic-{severity}"

            current_diag = {
                "file": file_path,
                "line": int(match.group(3)),
                "col": int(match.group(4)),
                "severity": severity,
                "message": message,
                "check": check_name,
                "lines": [line],
            }
            diagnostics.append(current_diag)
        elif current_diag:
            # Associated line (note, code snippet, etc.)
            current_diag["lines"].append(line)

    return diagnostics


def extract_rename_candidates(
    diagnostics: list[dict], check_name: str = "readability-identifier-naming"
) -> list[dict]:
    candidates = []
    for diag in diagnostics:
        if diag.get("check") != check_name:
            continue

        message = diag.get("message", "")
        invalid_match = INVALID_CASE_STYLE_PATTERN.search(message)
        if not invalid_match:
            continue

        symbol_kind = invalid_match.group(1).strip()
        old_name = invalid_match.group(2).strip()
        suggested_name = ""

        for line in diag.get("lines", [])[1:]:
            clean_line = _strip_ansi(line)
            suggestion_match = SUGGESTED_NAME_PATTERN.match(clean_line)
            if not suggestion_match:
                continue
            maybe_name = suggestion_match.group(1).strip()
            if maybe_name and maybe_name != old_name:
                suggested_name = maybe_name
                break

        if not suggested_name:
            continue

        candidates.append(
            {
                "file": diag.get("file", ""),
                "line": diag.get("line", 0),
                "col": diag.get("col", 0),
                "check": diag.get("check", ""),
                "symbol_kind": symbol_kind,
                "old_name": old_name,
                "new_name": suggested_name,
                "message": message,
            }
        )

    return candidates


def generate_text_summary(warnings: list[dict]) -> str:
    if not warnings:
        return ""
    file_counts = defaultdict(int)
    check_counts = defaultdict(int)
    for w in warnings:
        file_counts[w["file"].split("/")[-1]] += 1
        check_counts[w["check"]] += 1

    lines = [
        "=== SUMMARY ===",
        f"Files: {', '.join(f'{f}({c})' for f, c in sorted(file_counts.items(), key=lambda x: -x[1]))}",
        f"Types: {', '.join(f'{c}({n})' for c, n in sorted(check_counts.items(), key=lambda x: -x[1]))}",
        "=" * 15,
        "",
    ]
    return "\n".join(lines)
