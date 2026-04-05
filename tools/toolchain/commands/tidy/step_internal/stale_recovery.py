from __future__ import annotations

from pathlib import Path

from ....core.context import Context
from ...cmd_build import BuildCommand
from ..clean import CleanCommand


def can_continue_after_fix_failures(fix_result) -> bool:
    allowed_rename_failure_markers = (
        "Cannot rename symbol: there is no symbol at the given location",
        "Cannot rename symbol: symbol is not a supported kind",
    )
    if fix_result.failed <= 0:
        return True
    failed_actions = [action for action in fix_result.actions if action.status == "failed"]
    if not failed_actions:
        return False
    for action in failed_actions:
        reason = str(action.reason or "")
        if action.kind != "rename":
            return False
        if not any(marker in reason for marker in allowed_rename_failure_markers):
            return False
    return True


def handle_stale_task_before_fix(
    ctx: Context,
    *,
    app_name: str,
    task_ctx,
    verify_build_dir_name: str | None,
    profile_name: str | None,
    concise: bool,
    kill_build_procs: bool,
    workspace,
    resolved_task_path: Path,
    config_file: str | None,
    strict_config: bool,
    run_task_recheck_fn,
    build_queue_head_fn,
    sync_queue_snapshot_after_close_fn,
    rewrite_task_artifacts_from_recheck_fn,
) -> int | None:
    verify_ret = BuildCommand(ctx).build(
        app_name=app_name,
        tidy=False,
        build_dir_name=verify_build_dir_name,
        profile_name=profile_name,
        concise=concise,
        kill_build_procs=kill_build_procs,
    )
    if verify_ret != 0:
        print("--- tidy-step: build sanity check failed during stale-task recovery.")
        return verify_ret

    recheck_result = run_task_recheck_fn(
        app_name=app_name,
        parsed=task_ctx.parsed_task,
        tidy_build_dir_name=workspace.build_dir_name,
        source_scope=workspace.source_scope,
        config_file=config_file,
        strict_config=strict_config,
    )
    if recheck_result.ok:
        print(
            "--- tidy-step: stale task no longer has matching diagnostics; "
            "archiving outdated task artifact(s)."
        )
        clean_ret = CleanCommand(ctx).execute(
            app_name=app_name,
            task_ids=[task_ctx.parsed_task.task_id],
            batch_id=task_ctx.parsed_task.batch_id,
            tidy_build_dir_name=workspace.build_dir_name,
        )
        if clean_ret != 0:
            print("--- tidy-step: clean/archive failed after stale-task re-check.")
            return clean_ret
        queue_head_after_close = build_queue_head_fn(task_ctx.tasks_dir)
        sync_queue_snapshot_after_close_fn(
            app_name=app_name,
            tidy_build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            closed_batch_id=task_ctx.parsed_task.batch_id,
            closed_task_id=task_ctx.parsed_task.task_id,
            closed_task_path=resolved_task_path,
            recheck_log_path=recheck_result.log_path,
            queue_head_after_close=queue_head_after_close,
            config_file=config_file,
            strict_config=strict_config,
        )
        return 0

    if recheck_result.diagnostics:
        rewrite_task_artifacts_from_recheck_fn(
            task_ctx=task_ctx,
            diagnostics=list(recheck_result.diagnostics),
            resolved_task_path=resolved_task_path,
        )
        print(
            "--- tidy-step: stale task artifact refreshed from focused re-check; "
            "rerun the current queue head before applying fixes."
        )
        return 1

    print("--- tidy-step: stale task re-check failed before diagnostics could be refreshed.")
    return recheck_result.exit_code or 1

