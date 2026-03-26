from __future__ import annotations

from dataclasses import replace
from pathlib import Path

from .engines import ClangdRenameEngine, TextEditEngine
from .models import ExecutionRecord, FixContext, FixIntent
from .registry import build_default_registry
from ..task_auto_fix_report import report_paths, write_result_report
from ..task_auto_fix_types import AutoFixAction, TaskAutoFixResult
from ..task_log import parse_task_log, resolve_task_json_path
from ..workspace import resolve_workspace


def run_task_auto_fix_orchestrator(
    ctx,
    *,
    app_name: str,
    task_log_path: str | None = None,
    batch_id: str | None = None,
    task_id: str | None = None,
    tidy_build_dir_name: str | None = None,
    source_scope: str | None = None,
    dry_run: bool = False,
    report_suffix: str = "fix",
) -> TaskAutoFixResult:
    workspace = resolve_workspace(
        ctx,
        build_dir_name=tidy_build_dir_name,
        source_scope=source_scope,
    )
    tidy_layout = ctx.get_tidy_layout(app_name, workspace.build_dir_name)
    build_tidy_dir = tidy_layout.root
    tasks_dir = tidy_layout.tasks_dir
    resolved_task_path = resolve_task_json_path(
        tasks_dir,
        task_log_path=task_log_path,
        batch_id=batch_id,
        task_id=task_id,
    )
    parsed = parse_task_log(resolved_task_path)
    source_file = parsed.source_file or str(resolved_task_path)
    json_path, markdown_path = report_paths(
        build_tidy_dir,
        parsed.batch_id or "batch_unknown",
        parsed.task_id,
        report_suffix,
    )

    context = FixContext(
        ctx=ctx,
        app_name=app_name,
        workspace=workspace,
        parsed=parsed,
        build_tidy_dir=build_tidy_dir,
        dry_run=dry_run,
    )
    registry = build_default_registry()
    intents = registry.plan(context)
    records = _execute_intents(context, intents)
    actions = [_intent_record_to_action(intent, record) for intent, record in zip(intents, records)]

    result = TaskAutoFixResult(
        app_name=app_name,
        task_id=parsed.task_id,
        batch_id=parsed.batch_id,
        task_log=str(resolved_task_path),
        source_file=source_file,
        mode="dry_run" if dry_run else "apply",
        workspace=workspace.build_dir_name,
        source_scope=workspace.source_scope,
        action_count=len(actions),
        actions=actions,
        json_path=str(json_path),
        markdown_path=str(markdown_path),
    )
    for action in actions:
        if action.status == "applied":
            result.applied += 1
        elif action.status == "previewed":
            result.previewed += 1
        elif action.status == "failed":
            result.failed += 1
        else:
            result.skipped += 1

    write_result_report(result)
    return result


def _execute_intents(context: FixContext, intents: list[FixIntent]) -> list[ExecutionRecord]:
    if not intents:
        return []
    engines = {
        "clangd": ClangdRenameEngine(),
        "text": TextEditEngine(),
    }
    ordered_records: dict[str, ExecutionRecord] = {}
    intents_by_engine: dict[str, list[FixIntent]] = {}
    for intent in intents:
        intents_by_engine.setdefault(intent.engine_id, []).append(intent)
    for engine_id, engine_intents in intents_by_engine.items():
        engine = engines.get(engine_id)
        if engine is None:
            for intent in engine_intents:
                ordered_records[intent.intent_id] = ExecutionRecord(
                    intent_id=intent.intent_id,
                    status="failed",
                    reason=f"unsupported_engine:{engine_id}",
                )
            continue
        try:
            engine_records = engine.execute(context, engine_intents)
        except Exception as exc:
            failure_reason = (
                f"clangd_bootstrap_failed:{exc}" if engine_id == "clangd" else f"engine_execute_failed:{exc}"
            )
            for intent in engine_intents:
                ordered_records[intent.intent_id] = ExecutionRecord(
                    intent_id=intent.intent_id,
                    status="failed",
                    reason=failure_reason,
                    old_name=str(intent.payload.get("old_name", "")),
                    new_name=str(intent.payload.get("new_name", "")),
                    replacement=str(intent.payload.get("replacement", "")),
                )
            continue
        for record in engine_records:
            status = record.status
            if status == "previewed" and not context.dry_run:
                status = "applied"
            ordered_records[record.intent_id] = replace(record, status=status)
    return [
        ordered_records.get(
            intent.intent_id,
            ExecutionRecord(
                intent_id=intent.intent_id,
                status="failed",
                reason="missing_execution_record",
            ),
        )
        for intent in intents
    ]


def _intent_record_to_action(intent: FixIntent, record: ExecutionRecord) -> AutoFixAction:
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
