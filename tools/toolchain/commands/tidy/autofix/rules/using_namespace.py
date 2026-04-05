from __future__ import annotations

import re
from pathlib import Path

from ..analyzers.dto_symbol_index import build_dto_using_declarations, resolve_using_namespace_line
from ..models import FixContext, ReplaceLineWithBlockOp
from .base import RuleBase
from .catalog import USING_NAMESPACE_METADATA

_CPP_IMPLEMENTATION_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".cppm", ".ixx"}
_USING_NAMESPACE_ALLOWED_SEGMENTS = (
    "/application/use_cases/",
    "/application/pipeline/",
)
_DTO_USING_NAMESPACE_LINE = "using namespace tracer_core::core::dto;"
_CHRONO_USING_NAMESPACE_LINE = "using namespace std::chrono;"
_CHRONO_SYMBOLS = (
    "duration_cast",
    "duration",
    "milliseconds",
    "microseconds",
    "nanoseconds",
    "seconds",
    "minutes",
    "hours",
    "system_clock",
    "steady_clock",
    "high_resolution_clock",
    "time_point",
    "time_point_cast",
    "floor",
    "ceil",
    "round",
    "abs",
)


class UsingNamespaceRule(RuleBase):
    metadata = USING_NAMESPACE_METADATA

    def plan(self, context: FixContext, diagnostic) -> list:
        if not self.supports(diagnostic.check):
            return []
        source_file = Path(self.resolve_source_file(context, diagnostic))
        if not source_file.exists():
            return []
        if source_file.suffix.lower() not in _CPP_IMPLEMENTATION_SUFFIXES:
            return []
        normalized_source = str(source_file).replace("\\", "/").lower()
        if not any(segment in normalized_source for segment in _USING_NAMESPACE_ALLOWED_SEGMENTS):
            return []
        source_lines = source_file.read_text(encoding="utf-8", errors="replace").splitlines()
        using_line_index = _resolve_supported_using_namespace_line(source_lines, diagnostic.line)
        if using_line_index is None:
            return []
        using_line = source_lines[using_line_index].strip()
        using_declarations = _build_using_declarations(
            source_file=source_file,
            source_lines=source_lines,
            using_line=using_line,
        )
        if not using_declarations:
            return []
        if using_line == _CHRONO_USING_NAMESPACE_LINE:
            success_reason = "chrono_using_declarations_preview"
        else:
            success_reason = "dto_using_declarations_preview"
        return [
            self.build_intent(
                intent_id=f"using_namespace:{using_line_index + 1:03d}:{diagnostic.col:03d}",
                check=diagnostic.check,
                file_path=str(source_file),
                line=using_line_index + 1,
                col=max(1, source_lines[using_line_index].find("using namespace") + 1),
                operation=ReplaceLineWithBlockOp(
                    expected_line=using_line,
                    replacement_lines=tuple(using_declarations),
                    success_reason=success_reason,
                    missing_reason="using_namespace_directive_not_found",
                    empty_replacement_reason="no_safe_using_declarations_generated",
                ),
            )
        ]


def _resolve_supported_using_namespace_line(
    source_lines: list[str],
    diagnostic_line: int,
) -> int | None:
    direct_index = diagnostic_line - 1
    if 0 <= direct_index < len(source_lines):
        stripped = source_lines[direct_index].strip()
        if stripped in {_DTO_USING_NAMESPACE_LINE, _CHRONO_USING_NAMESPACE_LINE}:
            return direct_index
    dto_index = resolve_using_namespace_line(source_lines)
    if dto_index is not None:
        return dto_index
    for index, line in enumerate(source_lines):
        if line.strip() == _CHRONO_USING_NAMESPACE_LINE:
            return index
    return None


def _build_using_declarations(
    *,
    source_file: Path,
    source_lines: list[str],
    using_line: str,
) -> list[str]:
    if using_line == _DTO_USING_NAMESPACE_LINE:
        return build_dto_using_declarations(source_file, source_lines)
    if using_line == _CHRONO_USING_NAMESPACE_LINE:
        return _build_chrono_using_declarations(source_lines)
    return []


def _build_chrono_using_declarations(source_lines: list[str]) -> list[str]:
    existing_declarations = {
        line.strip()
        for line in source_lines
        if line.strip().startswith("using std::chrono::")
    }
    body_lines = [
        line
        for line in source_lines
        if line.strip() != _CHRONO_USING_NAMESPACE_LINE and not line.strip().startswith("#include")
    ]
    body = "\n".join(body_lines)
    positions: list[tuple[int, str]] = []
    for symbol in _CHRONO_SYMBOLS:
        found = re.search(rf"\b{re.escape(symbol)}\b", body)
        if found is None:
            continue
        positions.append((found.start(), symbol))
    positions.sort(key=lambda item: (item[0], item[1]))
    declarations: list[str] = []
    for _, symbol in positions:
        candidate = f"using std::chrono::{symbol};"
        if candidate in existing_declarations:
            continue
        if candidate in declarations:
            continue
        declarations.append(candidate)
    return declarations
