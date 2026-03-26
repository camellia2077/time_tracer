from __future__ import annotations

from pathlib import Path

from ..analyzers.dto_symbol_index import build_dto_using_declarations, resolve_using_namespace_line
from ..models import FixContext, FixIntent

_CPP_IMPLEMENTATION_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".cppm", ".ixx"}
_USING_NAMESPACE_ALLOWED_SEGMENT = "/application/use_cases/"
_DTO_USING_NAMESPACE_LINE = "using namespace tracer_core::core::dto;"


class UsingNamespaceRule:
    rule_id = "using_namespace"
    supported_checks = ("google-build-using-namespace",)
    engine_id = "text"
    preview_only = True

    def plan(self, context: FixContext, diagnostic) -> list[FixIntent]:
        if diagnostic.check != "google-build-using-namespace":
            return []
        source_file = Path(diagnostic.file or context.parsed.source_file)
        if not source_file.exists():
            return []
        if source_file.suffix.lower() not in _CPP_IMPLEMENTATION_SUFFIXES:
            return []
        normalized_source = str(source_file).replace("\\", "/").lower()
        if _USING_NAMESPACE_ALLOWED_SEGMENT not in normalized_source:
            return []
        source_lines = source_file.read_text(encoding="utf-8", errors="replace").splitlines()
        using_line_index = resolve_using_namespace_line(source_lines)
        if using_line_index is None:
            return []
        using_declarations = build_dto_using_declarations(source_file, source_lines)
        if not using_declarations:
            return []
        return [
            FixIntent(
                intent_id=f"using_namespace:{using_line_index + 1:03d}:{diagnostic.col:03d}",
                rule_id=self.rule_id,
                check=diagnostic.check,
                engine_id=self.engine_id,
                file_path=str(source_file),
                line=using_line_index + 1,
                col=max(1, source_lines[using_line_index].find("using namespace") + 1),
                payload={
                    "action_kind": "using_namespace",
                    "operation": "replace_line_with_block",
                    "old_name": _DTO_USING_NAMESPACE_LINE,
                    "replacement": "\n".join(using_declarations),
                },
                preview_only=self.preview_only,
            )
        ]
