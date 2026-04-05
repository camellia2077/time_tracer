from __future__ import annotations

from pathlib import Path

from ..models import FixContext, InsertPrefixOnLineOp
from .base import RuleBase
from .catalog import EXPLICIT_CONSTRUCTOR_METADATA

_CPP_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".cppm", ".ixx"}


class ExplicitConstructorRule(RuleBase):
    metadata = EXPLICIT_CONSTRUCTOR_METADATA

    def plan(self, context: FixContext, diagnostic) -> list:
        if not self.supports(diagnostic.check):
            return []
        source_file = Path(self.resolve_source_file(context, diagnostic))
        if not source_file.exists():
            return []
        if source_file.suffix.lower() not in _CPP_SUFFIXES:
            return []
        return [
            self.build_intent(
                intent_id=f"explicit_ctor:{diagnostic.line:03d}:{diagnostic.col:03d}",
                check=diagnostic.check,
                file_path=str(source_file),
                line=diagnostic.line,
                col=diagnostic.col,
                operation=InsertPrefixOnLineOp(
                    prefix="explicit ",
                    success_reason="google_explicit_constructor_added",
                    already_prefixed_reason="already_explicit",
                    no_match_reason="no_safe_constructor_signature_match",
                ),
            )
        ]
