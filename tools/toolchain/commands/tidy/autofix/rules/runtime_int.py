from __future__ import annotations

import re
from pathlib import Path

from ..models import FixContext, ReplaceLiteralOnLineOp
from .base import RuleBase
from .catalog import RUNTIME_INT_METADATA

_GOOGLE_RUNTIME_INT_PATTERN = re.compile(
    r"consider replacing '(?P<source>[^']+)' with '(?P<target>[^']+)'"
)
_CPP_IMPLEMENTATION_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".cppm", ".ixx"}
_GOOGLE_RUNTIME_INT_TARGETS = {
    ("long long", "int64"): "std::int64_t",
    ("long long", "int64_t"): "std::int64_t",
    ("long long", "std::int64_t"): "std::int64_t",
    ("unsigned long long", "uint64"): "std::uint64_t",
    ("unsigned long long", "uint64_t"): "std::uint64_t",
    ("unsigned long long", "std::uint64_t"): "std::uint64_t",
}


class RuntimeIntRule(RuleBase):
    metadata = RUNTIME_INT_METADATA

    def plan(self, context: FixContext, diagnostic) -> list:
        if not self.supports(diagnostic.check):
            return []
        source_file = Path(self.resolve_source_file(context, diagnostic))
        if not source_file.exists():
            return []
        if source_file.suffix.lower() not in _CPP_IMPLEMENTATION_SUFFIXES:
            return []
        old_type, new_type = runtime_int_replacement(diagnostic.message)
        if not old_type or not new_type:
            return []
        return [
            self.build_intent(
                intent_id=f"runtime_int:{diagnostic.line:03d}:{diagnostic.col:03d}",
                check=diagnostic.check,
                file_path=str(source_file),
                line=diagnostic.line,
                col=diagnostic.col,
                operation=ReplaceLiteralOnLineOp(
                    old_name=old_type,
                    new_name=new_type,
                    ensure_include="cstdint",
                    success_reason="google_runtime_int_replaced",
                    missing_reason="missing_runtime_int_payload",
                    no_match_reason="no_safe_same_line_runtime_int_match",
                    already_rewritten_reason="already_rewritten",
                ),
            )
        ]


def runtime_int_replacement(message: str) -> tuple[str, str]:
    match = _GOOGLE_RUNTIME_INT_PATTERN.search(message.strip())
    if not match:
        return "", ""
    source_text = match.group("source").strip()
    target_text = match.group("target").strip()
    replacement = _GOOGLE_RUNTIME_INT_TARGETS.get((source_text, target_text))
    if not replacement:
        return "", ""
    return source_text, replacement
