from __future__ import annotations

from pathlib import Path

from ..models import FixContext, FixIntent

_CPP_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".cppm", ".ixx"}


class ExplicitConstructorRule:
    rule_id = "explicit_constructor"
    supported_checks = ("google-explicit-constructor",)
    engine_id = "text"
    preview_only = False

    def plan(self, context: FixContext, diagnostic) -> list[FixIntent]:
        if diagnostic.check != "google-explicit-constructor":
            return []
        source_file = Path(diagnostic.file or context.parsed.source_file)
        if not source_file.exists():
            return []
        if source_file.suffix.lower() not in _CPP_SUFFIXES:
            return []
        return [
            FixIntent(
                intent_id=f"explicit_ctor:{diagnostic.line:03d}:{diagnostic.col:03d}",
                rule_id=self.rule_id,
                check=diagnostic.check,
                engine_id=self.engine_id,
                file_path=str(source_file),
                line=diagnostic.line,
                col=diagnostic.col,
                payload={
                    "action_kind": "explicit_constructor",
                    "operation": "insert_prefix_on_line",
                    "prefix": "explicit ",
                    "replacement": "explicit ",
                },
                preview_only=self.preview_only,
            )
        ]
