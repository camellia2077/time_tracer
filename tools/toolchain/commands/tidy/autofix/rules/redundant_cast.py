from __future__ import annotations

from pathlib import Path
import re

from ..models import FixContext, FixIntent

_REDUNDANT_CAST_PATTERN = re.compile(
    r"static_cast<(?P<target>[^>]+)>\((?P<expr>[^()]+)\)"
)


class RedundantCastRule:
    rule_id = "redundant_cast"
    supported_checks = ("readability-redundant-casting",)
    engine_id = "text"
    preview_only = False

    def plan(self, context: FixContext, diagnostic) -> list[FixIntent]:
        if diagnostic.check != "readability-redundant-casting":
            return []
        if "same type" not in diagnostic.message.strip():
            return []
        source_file = Path(diagnostic.file or context.parsed.source_file)
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
        replacement = selected_match.group("expr").strip()
        return [
            FixIntent(
                intent_id=f"cast:{diagnostic.line:03d}:{diagnostic.col:03d}",
                rule_id=self.rule_id,
                check=diagnostic.check,
                engine_id=self.engine_id,
                file_path=str(source_file),
                line=diagnostic.line,
                col=diagnostic.col,
                payload={
                    "action_kind": "redundant_cast",
                    "operation": "replace_redundant_cast_on_line",
                    "replacement": replacement,
                },
                preview_only=self.preview_only,
            )
        ]
