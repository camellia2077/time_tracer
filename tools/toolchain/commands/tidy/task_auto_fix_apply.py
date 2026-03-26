from __future__ import annotations

from pathlib import Path

from . import analysis_compile_db
from .autofix.engines.clangd_rename_engine import ClangdRenameEngine, allowed_roots
from .autofix.engines.text_edit_engine import TextEditEngine
from .autofix.models import ExecutionRecord, FixContext, FixIntent
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
            rule_id="redundant_cast",
            check=action.check,
            engine_id="text",
            file_path=action.file_path,
            line=action.line,
            col=action.col,
            payload={
                "action_kind": "redundant_cast",
                "operation": "replace_redundant_cast_on_line",
                "replacement": action.replacement,
            },
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
            rule_id="runtime_int",
            check=action.check,
            engine_id="text",
            file_path=action.file_path,
            line=action.line,
            col=action.col,
            payload={
                "action_kind": "runtime_int",
                "operation": "replace_literal_on_line",
                "old_name": action.old_name,
                "new_name": action.new_name or action.replacement,
                "replacement": action.replacement or action.new_name,
                "ensure_include": "cstdint",
            },
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
            rule_id="explicit_constructor",
            check=action.check,
            engine_id="text",
            file_path=action.file_path,
            line=action.line,
            col=action.col,
            payload={
                "action_kind": "explicit_constructor",
                "operation": "insert_prefix_on_line",
                "prefix": action.replacement or "explicit ",
                "replacement": action.replacement or "explicit ",
            },
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
    intents = [
        FixIntent(
            intent_id=action.action_id,
            rule_id="using_namespace",
            check=action.check,
            engine_id="text",
            file_path=action.file_path,
            line=action.line,
            col=action.col,
            payload={
                "action_kind": "using_namespace",
                "operation": "replace_line_with_block",
                "old_name": action.old_name,
                "replacement": action.replacement,
            },
            preview_only=True,
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
        rule_id="identifier_naming",
        check=str(candidate.get("check", "")),
        engine_id="clangd",
        file_path=str(candidate.get("file", "")),
        line=int(candidate.get("line", 0)),
        col=int(candidate.get("col", 0)),
        payload={
            "action_kind": "rename",
            "symbol_kind": str(candidate.get("symbol_kind", "")),
            "old_name": str(candidate.get("old_name", "")),
            "new_name": str(candidate.get("new_name", "")),
        },
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
        kind=str(intent.payload.get("action_kind", intent.rule_id)),
        file_path=intent.file_path,
        line=intent.line,
        col=intent.col,
        check=intent.check,
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
