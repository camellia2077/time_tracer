from __future__ import annotations

from pathlib import Path

from .. import analysis_compile_db
from ..autofix.engines.clangd_rename_engine import ClangdRenameEngine, allowed_roots
from ..autofix.engines.text_edit_engine import TextEditEngine
from ..autofix.models import (
    ExecutionRecord,
    FixContext,
    FixIntent,
    InsertPrefixOnLineOp,
    RenameSymbolOp,
    ReplaceLineWithBlockOp,
    ReplaceLiteralOnLineOp,
)
from ..autofix.rules import (
    EXPLICIT_CONSTRUCTOR_METADATA,
    IDENTIFIER_NAMING_METADATA,
    REDUNDANT_CAST_METADATA,
    RUNTIME_INT_METADATA,
    USING_NAMESPACE_METADATA,
)
from .task_auto_fix_types import AutoFixAction


def analysis_compile_db_dir(build_tidy_dir: Path) -> Path:
    return analysis_compile_db.ensure_analysis_compile_db(build_tidy_dir)


def apply_redundant_casts(
    actions: list[AutoFixAction],
    *,
    dry_run: bool,
) -> None:
    intents = [
        FixIntent(
            intent_id=action.action_id,
            check=action.check,
            file_path=action.file_path,
            line=action.line,
            col=action.col,
            metadata=REDUNDANT_CAST_METADATA,
            operation=ReplaceLiteralOnLineOp(
                old_name=action.old_name,
                new_name=action.replacement,
                success_reason="safe_same_type_cast_removed",
                missing_reason="missing_redundant_cast_payload",
                no_match_reason="no_safe_same_line_cast_match",
                already_rewritten_reason="already_rewritten",
            ),
        )
        for action in actions
        if action.kind == "redundant_cast"
    ]
    _apply_text_records(actions, intents, dry_run=dry_run)


def apply_runtime_int_actions(
    actions: list[AutoFixAction],
    *,
    dry_run: bool,
) -> None:
    intents = [
        FixIntent(
            intent_id=action.action_id,
            check=action.check,
            file_path=action.file_path,
            line=action.line,
            col=action.col,
            metadata=RUNTIME_INT_METADATA,
            operation=ReplaceLiteralOnLineOp(
                old_name=action.old_name,
                new_name=action.new_name or action.replacement,
                ensure_include="cstdint",
                success_reason="google_runtime_int_replaced",
                missing_reason="missing_runtime_int_payload",
                no_match_reason="no_safe_same_line_runtime_int_match",
                already_rewritten_reason="already_rewritten",
            ),
        )
        for action in actions
        if action.kind == "runtime_int"
    ]
    _apply_text_records(actions, intents, dry_run=dry_run)


def apply_explicit_constructor_actions(
    actions: list[AutoFixAction],
    *,
    dry_run: bool,
) -> None:
    intents = [
        FixIntent(
            intent_id=action.action_id,
            check=action.check,
            file_path=action.file_path,
            line=action.line,
            col=action.col,
            metadata=EXPLICIT_CONSTRUCTOR_METADATA,
            operation=InsertPrefixOnLineOp(
                prefix=action.replacement or "explicit ",
                success_reason="google_explicit_constructor_added",
                already_prefixed_reason="already_explicit",
                no_match_reason="no_safe_constructor_signature_match",
            ),
        )
        for action in actions
        if action.kind == "explicit_constructor"
    ]
    _apply_text_records(actions, intents, dry_run=dry_run)


def apply_using_namespace_actions(
    actions: list[AutoFixAction],
    *,
    dry_run: bool,
) -> None:
    def _using_namespace_reason(action: AutoFixAction) -> str:
        if action.reason:
            return action.reason
        if action.old_name.strip() == "using namespace std::chrono;":
            return "chrono_using_declarations_preview"
        return "dto_using_declarations_preview"

    intents = [
        FixIntent(
            intent_id=action.action_id,
            check=action.check,
            file_path=action.file_path,
            line=action.line,
            col=action.col,
            metadata=USING_NAMESPACE_METADATA,
            operation=ReplaceLineWithBlockOp(
                expected_line=action.old_name,
                replacement_lines=tuple(
                    line for line in action.replacement.splitlines() if line.strip()
                ),
                success_reason=_using_namespace_reason(action),
                missing_reason="using_namespace_directive_not_found",
                empty_replacement_reason="no_safe_using_declarations_generated",
            ),
            preview_only=USING_NAMESPACE_METADATA.preview_only,
        )
        for action in actions
        if action.kind == "using_namespace"
    ]
    _apply_text_records(actions, intents, dry_run=dry_run)


def rename_action(
    ctx,
    *,
    client,
    candidate: dict,
    allowed_roots: list[Path],
    dry_run: bool,
    action_index: int,
) -> AutoFixAction:
    intent = FixIntent(
        intent_id=f"rename:{action_index:03d}",
        check=str(candidate.get("check", "")),
        file_path=str(candidate.get("file", "")),
        line=int(candidate.get("line", 0)),
        col=int(candidate.get("col", 0)),
        metadata=IDENTIFIER_NAMING_METADATA,
        operation=RenameSymbolOp(
            symbol_kind=str(candidate.get("symbol_kind", "")),
            old_name=str(candidate.get("old_name", "")),
            new_name=str(candidate.get("new_name", "")),
            success_reason="supported_rule_driven_const_rename",
        ),
    )
    engine = ClangdRenameEngine()
    record = engine._execute_intent(
        FixContext(
            ctx=ctx,
            app_name="",
            workspace=None,
            parsed=None,
            build_tidy_dir=Path("."),
            dry_run=dry_run,
        ),
        client,
        allowed_roots,
        intent,
    )
    return _record_to_action(intent, record)


def _apply_text_records(
    actions: list[AutoFixAction],
    intents: list[FixIntent],
    *,
    dry_run: bool,
) -> None:
    if not intents:
        return
    engine = TextEditEngine()
    records = engine.execute(
        FixContext(
            ctx=None,
            app_name="",
            workspace=None,
            parsed=None,
            build_tidy_dir=Path("."),
            dry_run=dry_run,
        ),
        intents,
    )
    action_map = {action.action_id: action for action in actions}
    for intent, record in zip(intents, records):
        action = action_map.get(intent.intent_id)
        if action is None:
            continue
        _update_action(action, record)


def _update_action(action: AutoFixAction, record: ExecutionRecord) -> None:
    action.status = record.status
    action.reason = record.reason
    action.old_name = record.old_name or action.old_name
    action.new_name = record.new_name or action.new_name
    action.replacement = record.replacement or action.replacement
    action.diff = record.diff
    action.edit_count = record.edit_count
    action.changed_files = list(record.changed_files)


def _record_to_action(intent: FixIntent, record: ExecutionRecord) -> AutoFixAction:
    return AutoFixAction(
        action_id=intent.intent_id,
        kind=intent.action_kind,
        file_path=intent.file_path,
        line=intent.line,
        col=intent.col,
        check=intent.check,
        rule_id=intent.rule_id,
        risk_level=intent.risk_level,
        preview_only=intent.preview_only,
        status=record.status,
        reason=record.reason,
        old_name=record.old_name,
        new_name=record.new_name,
        replacement=record.replacement,
        diff=record.diff,
        edit_count=record.edit_count,
        changed_files=list(record.changed_files),
    )


__all__ = [
    "allowed_roots",
    "analysis_compile_db_dir",
    "apply_redundant_casts",
    "apply_runtime_int_actions",
    "apply_explicit_constructor_actions",
    "apply_using_namespace_actions",
    "rename_action",
]
