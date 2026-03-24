from __future__ import annotations

import re
from pathlib import Path

from .task_auto_fix_types import AutoFixAction

_SUPPORTED_RENAME_KINDS = {"constant", "variable"}
_EXACT_NAME_RULES = {
    "payload": "kPayload",
    "parsed": "kParsed",
    "ok": "kOk",
    "found": "kFound",
    "format": "kFormat",
    "root": "kRoot",
    "period": "kPeriod",
    "period_argument": "kPeriodArgument",
    "error_message": "kErrorMessage",
    "error_code": "kErrorCode",
    "error_category": "kErrorCategory",
}
_REDUNDANT_CAST_PATTERN = re.compile(
    r"static_cast<(?P<target>[^>]+)>\((?P<expr>[^()]+)\)"
)
_CPP_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}


def suggest_const_name(old_name: str) -> str:
    normalized = old_name.strip().strip("_")
    if not normalized:
        return ""
    if normalized in _EXACT_NAME_RULES:
        return _EXACT_NAME_RULES[normalized]

    parts = [part for part in normalized.split("_") if part]
    if not parts:
        return ""
    camel = "".join(part[:1].upper() + part[1:] for part in parts)
    return f"k{camel}"


def rename_candidates(parsed) -> list[dict]:
    from ...services import log_parser

    return log_parser.extract_rename_candidates(
        parsed.diagnostic_dicts(),
        check_name="readability-identifier-naming",
    )


def supported_rename_candidate(candidate: dict, file_path: Path, *, line_text: str) -> tuple[bool, str]:
    symbol_kind = str(candidate.get("symbol_kind", "")).strip().lower()
    if symbol_kind not in _SUPPORTED_RENAME_KINDS:
        return False, f"unsupported_symbol_kind:{symbol_kind or 'unknown'}"
    if file_path.suffix.lower() not in _CPP_SUFFIXES:
        return False, "unsupported_file_type"
    old_name = str(candidate.get("old_name", "")).strip()
    new_name = str(candidate.get("new_name", "")).strip()
    expected_name = suggest_const_name(old_name)
    if not old_name or not new_name:
        return False, "missing_rename_payload"
    if expected_name and new_name != expected_name:
        return False, "rename_not_rule_driven"
    if "const " not in line_text and "const auto" not in line_text and "const json" not in line_text:
        return False, "not_const_local_pattern"
    return True, "supported_rule_driven_const_rename"


def plan_redundant_cast_actions(parsed) -> list[AutoFixAction]:
    source_file = Path(parsed.source_file) if parsed.source_file else None
    if source_file is None or not source_file.exists():
        return []

    actions: list[AutoFixAction] = []
    source_lines = source_file.read_text(encoding="utf-8", errors="replace").splitlines()
    for index, diagnostic in enumerate(parsed.diagnostics, 1):
        if diagnostic.check != "readability-redundant-casting":
            continue
        if "same type" not in diagnostic.message.strip():
            continue
        line_number = int(diagnostic.line)
        col_number = int(diagnostic.col)
        if line_number <= 0 or line_number > len(source_lines):
            continue
        source_line = source_lines[line_number - 1]
        matches = list(_REDUNDANT_CAST_PATTERN.finditer(source_line))
        if not matches:
            continue
        selected_match = None
        source_index = max(0, col_number - 1)
        for match in matches:
            if match.start() <= source_index <= match.end():
                selected_match = match
                break
        if selected_match is None:
            selected_match = matches[0]
        replacement = selected_match.group("expr").strip()
        actions.append(
            AutoFixAction(
                action_id=f"cast:{index:03d}",
                kind="redundant_cast",
                file_path=str(source_file),
                line=line_number,
                col=col_number,
                check="readability-redundant-casting",
                replacement=replacement,
            )
        )
    return actions
