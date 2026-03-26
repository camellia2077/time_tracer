from __future__ import annotations

import re
from pathlib import Path

from ..analyzers import line_text
from ..models import FixContext, FixIntent

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
_CPP_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".cppm", ".ixx"}
_INVALID_CASE_STYLE_PATTERN = re.compile(r"invalid case style for ([^']+) '([^']+)'")
_SUGGESTED_NAME_PATTERN = re.compile(r"^\s*\|\s+([A-Za-z_][A-Za-z0-9_]*)\s*$")


class IdentifierNamingRule:
    rule_id = "identifier_naming"
    supported_checks = ("readability-identifier-naming",)
    engine_id = "clangd"
    preview_only = False

    def plan(self, context: FixContext, diagnostic) -> list[FixIntent]:
        candidate = diagnostic_to_candidate(diagnostic, default_file=context.parsed.source_file)
        if candidate is None:
            return []
        file_path = Path(str(candidate["file"]))
        if not file_path.exists():
            return []
        supported, _reason = supported_rename_candidate(
            candidate,
            file_path,
            line_text=line_text(file_path, int(candidate["line"])),
        )
        if not supported:
            return []
        return [
            FixIntent(
                intent_id=f"rename:{int(candidate['line']):03d}:{int(candidate['col']):03d}",
                rule_id=self.rule_id,
                check=str(candidate["check"]),
                engine_id=self.engine_id,
                file_path=str(candidate["file"]),
                line=int(candidate["line"]),
                col=int(candidate["col"]),
                payload={
                    "action_kind": "rename",
                    "symbol_kind": str(candidate["symbol_kind"]),
                    "old_name": str(candidate["old_name"]),
                    "new_name": str(candidate["new_name"]),
                },
                preview_only=self.preview_only,
            )
        ]


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


def extract_rename_candidates(parsed) -> list[dict]:
    candidates: list[dict] = []
    for diagnostic in parsed.diagnostics:
        candidate = diagnostic_to_candidate(diagnostic, default_file=parsed.source_file)
        if candidate is not None:
            candidates.append(candidate)
    return candidates


def diagnostic_to_candidate(diagnostic, *, default_file: str) -> dict | None:
    if diagnostic.check != "readability-identifier-naming":
        return None
    match = _INVALID_CASE_STYLE_PATTERN.search(diagnostic.message)
    if not match:
        return None
    symbol_kind = match.group(1).strip().lower()
    old_name = match.group(2).strip()
    if not old_name:
        return None

    suggested_name = ""
    for raw_line in diagnostic.raw_lines[1:]:
        suggestion_match = _SUGGESTED_NAME_PATTERN.match(raw_line)
        if not suggestion_match:
            continue
        maybe_name = suggestion_match.group(1).strip()
        if maybe_name and maybe_name != old_name:
            suggested_name = maybe_name
            break
    if not suggested_name:
        suggested_name = suggest_const_name(old_name)
    if not suggested_name:
        return None

    return {
        "file": diagnostic.file or default_file,
        "line": diagnostic.line,
        "col": diagnostic.col,
        "check": diagnostic.check,
        "symbol_kind": symbol_kind,
        "old_name": old_name,
        "new_name": suggested_name,
        "message": diagnostic.message,
    }


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
