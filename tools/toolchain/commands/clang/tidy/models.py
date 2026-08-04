from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True, slots=True)
class ClangTidyInvocationResult:
    """Structured result of one clang-tidy process invocation."""

    source_file: Path
    exit_code: int
    diagnostics: tuple[dict, ...]
    version: int = 1

    def to_dict(self) -> dict:
        return {
            "version": self.version,
            "source_file": str(self.source_file),
            "exit_code": self.exit_code,
            "diagnostics": [dict(item) for item in self.diagnostics],
        }
