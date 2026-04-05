from __future__ import annotations

import re
from pathlib import Path

from ..models import FixContext, ReplaceLiteralOnLineOp
from .base import RuleBase
from .catalog import CONCISE_PREPROCESSOR_METADATA

_CPP_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".cppm", ".ixx"}
_SUGGESTED_DIRECTIVE_PATTERN = re.compile(r"using\s+'#(?P<directive>ifdef|ifndef)'")
_IF_DEFINED_PATTERN = re.compile(
    r"^\s*#\s*if\s+defined\s*(?:\(\s*(?P<macro1>[A-Za-z_][A-Za-z0-9_]*)\s*\)|(?P<macro2>[A-Za-z_][A-Za-z0-9_]*))\s*(?P<trail>//.*)?$"
)
_IF_NOT_DEFINED_PATTERN = re.compile(
    r"^\s*#\s*if\s*!\s*defined\s*(?:\(\s*(?P<macro1>[A-Za-z_][A-Za-z0-9_]*)\s*\)|(?P<macro2>[A-Za-z_][A-Za-z0-9_]*))\s*(?P<trail>//.*)?$"
)


class ConcisePreprocessorDirectivesRule(RuleBase):
    metadata = CONCISE_PREPROCESSOR_METADATA

    def plan(self, context: FixContext, diagnostic) -> list:
        if not self.supports(diagnostic.check):
            return []
        source_file = Path(self.resolve_source_file(context, diagnostic))
        if not source_file.exists():
            return []
        if source_file.suffix.lower() not in _CPP_SUFFIXES:
            return []
        source_lines = source_file.read_text(encoding="utf-8", errors="replace").splitlines()
        line_index = diagnostic.line - 1
        if line_index < 0 or line_index >= len(source_lines):
            return []
        source_line = source_lines[line_index]
        old_text = source_line.strip()
        replacement = build_concise_preprocessor_line(source_line, diagnostic.message)
        if not old_text or not replacement or replacement == old_text:
            return []
        return [
            self.build_intent(
                intent_id=f"concise_pp:{diagnostic.line:03d}:{diagnostic.col:03d}",
                check=diagnostic.check,
                file_path=str(source_file),
                line=diagnostic.line,
                col=max(1, source_line.find("#") + 1),
                operation=ReplaceLiteralOnLineOp(
                    old_name=old_text,
                    new_name=replacement,
                    success_reason="concise_preprocessor_directive_rewritten",
                    missing_reason="missing_concise_preprocessor_payload",
                    no_match_reason="no_safe_preprocessor_match",
                    already_rewritten_reason="already_concise_preprocessor",
                ),
            )
        ]


def build_concise_preprocessor_line(source_line: str, message: str) -> str:
    suggested = _extract_suggested_directive(message)
    if suggested == "ifndef":
        parsed = _parse_if_not_defined(source_line)
        if parsed is None:
            return ""
        macro, tail = parsed
        return _compose_directive_line("ifndef", macro, tail)

    parsed = _parse_if_defined(source_line)
    if parsed is None:
        return ""
    macro, tail = parsed
    return _compose_directive_line("ifdef", macro, tail)


def _extract_suggested_directive(message: str) -> str:
    match = _SUGGESTED_DIRECTIVE_PATTERN.search(message.strip())
    if not match:
        return "ifdef"
    return str(match.group("directive") or "ifdef").lower()


def _parse_if_defined(source_line: str) -> tuple[str, str] | None:
    match = _IF_DEFINED_PATTERN.match(source_line)
    if match is None:
        return None
    macro = str(match.group("macro1") or match.group("macro2") or "").strip()
    if not macro:
        return None
    trail = str(match.group("trail") or "").rstrip()
    return macro, trail


def _parse_if_not_defined(source_line: str) -> tuple[str, str] | None:
    match = _IF_NOT_DEFINED_PATTERN.match(source_line)
    if match is None:
        return None
    macro = str(match.group("macro1") or match.group("macro2") or "").strip()
    if not macro:
        return None
    trail = str(match.group("trail") or "").rstrip()
    return macro, trail


def _compose_directive_line(kind: str, macro: str, trail: str) -> str:
    line = f"#{kind} {macro}"
    if trail:
        line += " " + trail
    return line
