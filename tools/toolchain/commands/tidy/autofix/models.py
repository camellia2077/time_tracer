from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any, Literal, Protocol


@dataclass(frozen=True, slots=True)
class FixContext:
    ctx: Any
    app_name: str
    workspace: Any
    parsed: Any
    build_tidy_dir: Path
    dry_run: bool


RuleRiskLevel = Literal["low", "medium", "high"]


@dataclass(frozen=True, slots=True)
class RuleMetadata:
    rule_id: str
    action_kind: str
    engine_id: str
    supported_checks: tuple[str, ...]
    preview_only: bool
    risk_level: RuleRiskLevel


@dataclass(frozen=True, slots=True)
class ReplaceLiteralOnLineOp:
    old_name: str
    new_name: str
    ensure_include: str = ""
    success_reason: str = "literal_replaced"
    missing_reason: str = "missing_replace_literal_payload"
    no_match_reason: str = "no_safe_same_line_literal_match"
    already_rewritten_reason: str = "already_rewritten"


@dataclass(frozen=True, slots=True)
class ReplaceLineWithBlockOp:
    expected_line: str
    replacement_lines: tuple[str, ...]
    success_reason: str = "line_replaced_with_block"
    missing_reason: str = "line_not_found"
    empty_replacement_reason: str = "empty_replacement_block"


@dataclass(frozen=True, slots=True)
class InsertPrefixOnLineOp:
    prefix: str
    success_reason: str = "prefix_inserted"
    already_prefixed_reason: str = "already_prefixed"
    no_match_reason: str = "no_safe_insert_point"


@dataclass(frozen=True, slots=True)
class RenameSymbolOp:
    symbol_kind: str
    old_name: str
    new_name: str
    success_reason: str = "rename_applied"


FixOperation = ReplaceLiteralOnLineOp | ReplaceLineWithBlockOp | InsertPrefixOnLineOp | RenameSymbolOp


@dataclass(frozen=True, slots=True)
class FixIntent:
    intent_id: str
    check: str
    file_path: str
    line: int
    col: int
    metadata: RuleMetadata
    operation: FixOperation
    preview_only: bool = False

    @property
    def rule_id(self) -> str:
        return self.metadata.rule_id

    @property
    def engine_id(self) -> str:
        return self.metadata.engine_id

    @property
    def action_kind(self) -> str:
        return self.metadata.action_kind

    @property
    def risk_level(self) -> RuleRiskLevel:
        return self.metadata.risk_level


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
    metadata: RuleMetadata
    rule_id: str
    action_kind: str
    supported_checks: tuple[str, ...]
    engine_id: str
    preview_only: bool
    risk_level: RuleRiskLevel

    def plan(self, context: FixContext, diagnostic: Any) -> list[FixIntent]:
        ...


class AutoFixEngine(Protocol):
    engine_id: str

    def execute(self, context: FixContext, intents: list[FixIntent]) -> list[ExecutionRecord]:
        ...


def operation_old_name(operation: FixOperation) -> str:
    if isinstance(operation, ReplaceLiteralOnLineOp):
        return operation.old_name
    if isinstance(operation, ReplaceLineWithBlockOp):
        return operation.expected_line
    if isinstance(operation, RenameSymbolOp):
        return operation.old_name
    return ""


def operation_new_name(operation: FixOperation) -> str:
    if isinstance(operation, ReplaceLiteralOnLineOp):
        return operation.new_name
    if isinstance(operation, RenameSymbolOp):
        return operation.new_name
    return ""


def operation_replacement(operation: FixOperation) -> str:
    if isinstance(operation, ReplaceLiteralOnLineOp):
        return operation.new_name
    if isinstance(operation, ReplaceLineWithBlockOp):
        return "\n".join(operation.replacement_lines)
    if isinstance(operation, InsertPrefixOnLineOp):
        return operation.prefix
    return ""
