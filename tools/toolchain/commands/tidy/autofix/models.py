from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Protocol


@dataclass(frozen=True, slots=True)
class FixContext:
    ctx: Any
    app_name: str
    workspace: Any
    parsed: Any
    build_tidy_dir: Path
    dry_run: bool


@dataclass(frozen=True, slots=True)
class FixIntent:
    intent_id: str
    rule_id: str
    check: str
    engine_id: str
    file_path: str
    line: int
    col: int
    payload: dict[str, Any] = field(default_factory=dict)
    preview_only: bool = False


@dataclass(frozen=True, slots=True)
class WorkspaceTextEdit:
    file_path: str
    start_offset: int
    end_offset: int
    new_text: str


@dataclass(frozen=True, slots=True)
class ExecutionRecord:
    intent_id: str
    status: str
    reason: str
    diff: str = ""
    edit_count: int = 0
    changed_files: tuple[str, ...] = ()
    old_name: str = ""
    new_name: str = ""
    replacement: str = ""


class AutoFixRule(Protocol):
    rule_id: str
    supported_checks: tuple[str, ...]
    engine_id: str
    preview_only: bool

    def plan(self, context: FixContext, diagnostic: Any) -> list[FixIntent]:
        ...


class AutoFixEngine(Protocol):
    engine_id: str

    def execute(self, context: FixContext, intents: list[FixIntent]) -> list[ExecutionRecord]:
        ...
