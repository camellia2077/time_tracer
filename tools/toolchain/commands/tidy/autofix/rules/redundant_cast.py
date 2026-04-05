from __future__ import annotations

import re
from pathlib import Path

from ..models import FixContext, ReplaceLiteralOnLineOp
from .base import RuleBase
from .catalog import REDUNDANT_CAST_METADATA

_REDUNDANT_CAST_PATTERN = re.compile(
    r"static_cast<(?P<target>[^>]+)>\((?P<expr>[^()]+)\)"
)


class RedundantCastRule(RuleBase):
    metadata = REDUNDANT_CAST_METADATA

    def plan(self, context: FixContext, diagnostic) -> list:
        if not self.supports(diagnostic.check):
            return []
        if "same type" not in diagnostic.message.strip():
            return []
        source_file = Path(self.resolve_source_file(context, diagnostic))
        if not source_file.exists():
            return []
        source_lines = source_file.read_text(encoding="utf-8", errors="replace").splitlines()
        line_number = int(diagnostic.line)
        if line_number <= 0 or line_number > len(source_lines):
            return []
        source_line = source_lines[line_number - 1]
        matches = list(_REDUNDANT_CAST_PATTERN.finditer(source_line))
        if not matches:
            return []
        source_index = max(0, int(diagnostic.col) - 1)
        selected_match = None
        for match in matches:
            if match.start() <= source_index <= match.end():
                selected_match = match
                break
        if selected_match is None:
            selected_match = matches[0]
        old_text = selected_match.group(0).strip()
        replacement = selected_match.group("expr").strip()
        return [
            self.build_intent(
                intent_id=f"cast:{diagnostic.line:03d}:{diagnostic.col:03d}",
                check=diagnostic.check,
                file_path=str(source_file),
                line=diagnostic.line,
                col=diagnostic.col,
                operation=ReplaceLiteralOnLineOp(
                    old_name=old_text,
                    new_name=replacement,
                    success_reason="safe_same_type_cast_removed",
                    missing_reason="missing_redundant_cast_payload",
                    no_match_reason="no_safe_same_line_cast_match",
                    already_rewritten_reason="already_rewritten",
                ),
            )
        ]
