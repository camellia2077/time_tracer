from __future__ import annotations

from ...services.clangd_lsp import ClangdClient
from .task_auto_fix_apply import (
    allowed_roots,
    analysis_compile_db_dir,
    apply_redundant_casts,
    detect_task_refactors,
    rename_action,
)
from .task_auto_fix_plan import plan_redundant_cast_actions, rename_candidates, suggest_const_name
from . import task_auto_fix_report
from .task_auto_fix_types import AutoFixAction, TaskAutoFixResult
from .task_log import parse_task_log, resolve_task_log_path
from .workspace import resolve_workspace


def run_task_auto_fix(
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
    resolved_task_path = resolve_task_log_path(
        tasks_dir,
        task_log_path=task_log_path,
        batch_id=batch_id,
        task_id=task_id,
    )
    parsed = parse_task_log(resolved_task_path)
    source_file = parsed.source_file or str(resolved_task_path)
    json_path, markdown_path = task_auto_fix_report.report_paths(
        build_tidy_dir,
        parsed.batch_id or "batch_unknown",
        parsed.task_id,
        report_suffix,
    )

    result = TaskAutoFixResult(
        app_name=app_name,
        task_id=parsed.task_id,
        batch_id=parsed.batch_id,
        task_log=str(resolved_task_path),
        source_file=source_file,
        mode="dry_run" if dry_run else "apply",
        workspace=workspace.build_dir_name,
        source_scope=workspace.source_scope,
        json_path=str(json_path),
        markdown_path=str(markdown_path),
    )

    rename_items = rename_candidates(parsed)
    cast_actions = plan_redundant_cast_actions(parsed)
    result.action_count = len(rename_items) + len(cast_actions)

    if rename_items:
        try:
            compile_db_dir = analysis_compile_db_dir(build_tidy_dir)
            allowed = allowed_roots(ctx, app_name, workspace)
            client = ClangdClient(
                clangd_path=ctx.config.rename.clangd_path,
                compile_commands_dir=compile_db_dir,
                root_dir=ctx.repo_root,
                background_index=ctx.config.rename.clangd_background_index,
            )
            client.start()
            try:
                for index, candidate in enumerate(rename_items, 1):
                    action = rename_action(
                        ctx,
                        client=client,
                        candidate=candidate,
                        allowed_roots=allowed,
                        dry_run=dry_run,
                        action_index=index,
                    )
                    result.actions.append(action)
            finally:
                client.stop()
        except Exception as exc:
            result.actions.append(
                AutoFixAction(
                    action_id="rename:bootstrap",
                    kind="rename",
                    file_path=source_file,
                    line=0,
                    col=0,
                    check="readability-identifier-naming",
                    status="failed",
                    reason=f"clangd_bootstrap_failed:{exc}",
                )
            )

    if cast_actions:
        apply_redundant_casts(cast_actions, dry_run=dry_run)
        result.actions.extend(cast_actions)

    for action in result.actions:
        if action.status == "applied":
            result.applied += 1
        elif action.status == "previewed":
            result.previewed += 1
        elif action.status == "failed":
            result.failed += 1
        else:
            result.skipped += 1

    task_auto_fix_report.write_result_report(result)
    return result


def suggest_task_refactors(parsed) -> list[dict]:
    return task_auto_fix_report.suggest_task_refactors(
        parsed,
        detect_task_refactors_fn=detect_task_refactors,
    )


def write_task_suggestion_report(**kwargs):
    return task_auto_fix_report.write_task_suggestion_report(**kwargs)


__all__ = [
    "AutoFixAction",
    "TaskAutoFixResult",
    "suggest_const_name",
    "run_task_auto_fix",
    "suggest_task_refactors",
    "write_task_suggestion_report",
    "detect_task_refactors",
]
