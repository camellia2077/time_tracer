from __future__ import annotations

from pathlib import Path

from ..models import FixContext, ReplaceLineWithBlockOp
from .base import RuleBase
from .catalog import BRACES_AROUND_STATEMENTS_METADATA

_CPP_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".cppm", ".ixx"}
_CONTROL_KEYWORDS = ("if", "for", "while")


class BracesAroundStatementsRule(RuleBase):
    metadata = BRACES_AROUND_STATEMENTS_METADATA

    def plan(self, context: FixContext, diagnostic) -> list:
        if not self.supports(diagnostic.check):
            return []
        source_file = Path(self.resolve_source_file(context, diagnostic))
        if not source_file.exists() or source_file.suffix.lower() not in _CPP_SUFFIXES:
            return []
        source_lines = source_file.read_text(encoding="utf-8", errors="replace").splitlines()
        line_index = diagnostic.line - 1
        if line_index < 0 or line_index >= len(source_lines):
            return []
        source_line = source_lines[line_index]
        replacement_lines = build_braced_statement(source_line, diagnostic.raw_lines)
        if replacement_lines is None:
            return []
        return [
            self.build_intent(
                intent_id=f"braces:{diagnostic.line:03d}:{diagnostic.col:03d}",
                check=diagnostic.check,
                file_path=str(source_file),
                line=diagnostic.line,
                col=diagnostic.col,
                operation=ReplaceLineWithBlockOp(
                    expected_line=source_line,
                    replacement_lines=replacement_lines,
                    match_mode="braces_single_line",
                    success_reason="braces_around_statement_added",
                    missing_reason="braces_source_line_changed",
                    empty_replacement_reason="empty_braces_replacement",
                ),
            )
        ]


def build_braced_statement(
    source_line: str,
    snippet_notes: tuple[str, ...] | list[str],
) -> tuple[str, ...] | None:
    """Build a conservative three-line replacement for a one-line control body."""

    if not any(note.strip().endswith("{") for note in snippet_notes):
        return None
    stripped = source_line.strip()
    if not stripped or "{" in stripped or "}" in stripped:
        return None
    keyword = next(
        (candidate for candidate in _CONTROL_KEYWORDS if stripped.startswith(candidate)),
        None,
    )
    if keyword is None or len(stripped) > len(keyword) and stripped[len(keyword)].isalnum():
        return None
    close_paren = _find_control_close_paren(stripped, stripped.find("("))
    if close_paren is None:
        return None
    head = stripped[: close_paren + 1].rstrip()
    body = stripped[close_paren + 1 :].strip()
    if not body or body.startswith("else") or ";" not in body or "//" in body and body.index("//") < body.find(";"):
        return None
    if body.count(";") != 1 or not body.endswith(";"):
        return None
    return head + " {", "  " + body, "}"


def _find_control_close_paren(text: str, open_paren: int) -> int | None:
    if open_paren < 0:
        return None
    depth = 0
    for index in range(open_paren, len(text)):
        char = text[index]
        if char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
            if depth == 0:
                return index
            if depth < 0:
                return None
    return None
