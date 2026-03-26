from __future__ import annotations

from pathlib import Path
import re

from ..models import FixContext, FixIntent

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


class RuntimeIntRule:
    rule_id = "runtime_int"
    supported_checks = ("google-runtime-int",)
    engine_id = "text"
    preview_only = False

    def plan(self, context: FixContext, diagnostic) -> list[FixIntent]:
        if diagnostic.check != "google-runtime-int":
            return []
        source_file = Path(diagnostic.file or context.parsed.source_file)
        if not source_file.exists():
            return []
        if source_file.suffix.lower() not in _CPP_IMPLEMENTATION_SUFFIXES:
            return []
        old_type, new_type = runtime_int_replacement(diagnostic.message)
        if not old_type or not new_type:
            return []
        return [
            FixIntent(
                intent_id=f"runtime_int:{diagnostic.line:03d}:{diagnostic.col:03d}",
                rule_id=self.rule_id,
                check=diagnostic.check,
                engine_id=self.engine_id,
                file_path=str(source_file),
                line=diagnostic.line,
                col=diagnostic.col,
                payload={
                    "action_kind": "runtime_int",
                    "operation": "replace_literal_on_line",
                    "old_name": old_type,
                    "new_name": new_type,
                    "replacement": new_type,
                    "ensure_include": "cstdint",
                },
                preview_only=self.preview_only,
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
