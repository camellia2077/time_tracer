from __future__ import annotations

from pathlib import Path

from ...core.context import Context
from ...core.executor import run_command
from ...services import log_parser
from ..cmd_build import BuildCommand
from ..shared import tidy as tidy_shared
from . import clang_tidy_config
from .clean import CleanCommand
from .refresh_internal import refresh_runner as tidy_refresh_runner
from .step_internal.queue_state import (
    build_close_transition_summary,
    build_queue_head,
    build_single_task_close_next_action,
    sync_queue_snapshot_after_close,
)
from .step_internal.recheck import (
    TaskRecheckResult,
    collect_recheck_files,
    match_remaining_diagnostics,
    path_key,
    rewrite_task_artifacts_from_recheck,
    run_task_recheck,
)
from .step_internal.stale_recovery import (
    can_continue_after_fix_failures,
    handle_stale_task_before_fix,
)
from .tasking.task_auto_fix import run_task_auto_fix
from .tasking.task_context import resolve_task_context
from .tasking.task_log import list_task_paths
from .workspace import resolve_workspace


class TidyStepCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        *,
        task_log_path: str,
        verify_build_dir_name: str | None = None,
        profile_name: str | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
        dry_run: bool = False,
        strict: bool = False,
        config_file: str | None = None,
        strict_config: bool = False,
    ) -> int:
        task_ctx = resolve_task_context(self.ctx, task_log_path=task_log_path)
        workspace = resolve_workspace(
            self.ctx,
            build_dir_name=task_ctx.tidy_build_dir_name,
            source_scope=task_ctx.source_scope,
        )
        app_name = task_ctx.app_name
        tidy_layout = self.ctx.get_tidy_layout(app_name, workspace.build_dir_name)
        tasks_dir = task_ctx.tasks_dir
        resolved_task_path = task_ctx.task_json_path
        parsed = task_ctx.parsed_task
        batch_tasks_before = list_task_paths(tasks_dir, batch_id=parsed.batch_id)
        print(
            f"--- tidy-step: selected {parsed.batch_id}/task_{parsed.task_id} "
            f"({parsed.source_file or resolved_task_path})"
        )

        if not task_ctx.source_fingerprint_matches:
            print(
                "--- tidy-step: source fingerprint drift detected; "
                "running build sanity + focused re-check before trusting task artifacts."
            )
            if dry_run:
                print("--- tidy-step: dry-run mode, skip stale-task recovery.")
                return 0

            stale_ret = self._handle_stale_task_before_fix(
                app_name=app_name,
                task_ctx=task_ctx,
                verify_build_dir_name=verify_build_dir_name,
                profile_name=profile_name,
                concise=concise,
                kill_build_procs=kill_build_procs,
                workspace=workspace,
                resolved_task_path=resolved_task_path,
                config_file=config_file,
                strict_config=strict_config,
            )
            if stale_ret is not None:
                return stale_ret

        fix_result = run_task_auto_fix(
            self.ctx,
            task_log_path=str(resolved_task_path),
            dry_run=dry_run,
            report_suffix="step",
        )
        print(
            f"--- tidy-task-fix: task={fix_result.task_id}, applied={fix_result.applied}, "
            f"previewed={fix_result.previewed}, skipped={fix_result.skipped}, failed={fix_result.failed}"
        )
        print(f"--- tidy-task-fix: report json -> {fix_result.json_path}")
        print(f"--- tidy-task-fix: report md   -> {fix_result.markdown_path}")
        if fix_result.failed > 0:
            if self._can_continue_after_fix_failures(fix_result):
                print(
                    "--- tidy-step: task auto-fix hit stale rename failures only; "
                    "continuing with verify/re-check for manual-close path."
                )
            else:
                print("--- tidy-step: task auto-fix failed.")
                return fix_result.exit_code(strict=strict)
        if fix_result.applied == 0 and fix_result.previewed == 0:
            print(
                "--- tidy-step: task auto-fix produced no supported edits; "
                "continuing with verify/re-check for manual-close path."
            )

        if dry_run:
            print("--- tidy-step: dry-run mode, skip verify/batch follow-up.")
            return 0

        print("--- tidy-step: running build sanity check...")
        verify_ret = BuildCommand(self.ctx).build(
            app_name=app_name,
            tidy=False,
            build_dir_name=verify_build_dir_name,
            profile_name=profile_name,
            concise=concise,
            kill_build_procs=kill_build_procs,
        )
        if verify_ret != 0:
            print("--- tidy-step: build sanity check failed.")
            return verify_ret

        print("--- tidy-step: running post-build clang-tidy re-check...")
        recheck_result = self._run_task_recheck(
            app_name=app_name,
            parsed=parsed,
            tidy_build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            config_file=config_file,
            strict_config=strict_config,
        )
        if not recheck_result.ok:
            print("--- tidy-step: task re-check failed or matching diagnostics remain.")
            print(f"--- tidy-step: re-check log -> {recheck_result.log_path}")
            for diagnostic in recheck_result.remaining_diagnostics[:5]:
                location = (
                    f"{diagnostic.get('file', parsed.source_file)}:"
                    f"{diagnostic.get('line', 0)}:{diagnostic.get('col', 0)}"
                )
                print(
                    "  - "
                    + f"{location} [{diagnostic.get('check', 'unknown')}] "
                    + str(diagnostic.get("message", "")).strip()
                )
            return recheck_result.exit_code or 1

        print("--- tidy-step: task re-check passed. Archiving task artifact(s)...")
        clean_ret = CleanCommand(self.ctx).execute(
            app_name=app_name,
            task_ids=[parsed.task_id],
            batch_id=parsed.batch_id,
            tidy_build_dir_name=workspace.build_dir_name,
        )
        if clean_ret != 0:
            print("--- tidy-step: clean/archive failed after successful re-check.")
            return clean_ret

        queue_head_after_close = self._build_queue_head(tasks_dir)
        self._sync_queue_snapshot_after_close(
            app_name=app_name,
            tidy_build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            closed_batch_id=parsed.batch_id,
            closed_task_id=parsed.task_id,
            closed_task_path=resolved_task_path,
            recheck_log_path=recheck_result.log_path,
            queue_head_after_close=queue_head_after_close,
            config_file=config_file,
            strict_config=strict_config,
        )
        if len(batch_tasks_before) == 1:
            next_action = self._build_single_task_close_next_action(
                batch_id=parsed.batch_id,
                task_id=parsed.task_id,
                queue_head=queue_head_after_close,
            )
            print(f"--- tidy-step: {next_action}")
            step_state_path = tidy_layout.automation_dir / "tidy_step_last.json"
            tidy_shared.write_json_dict(
                step_state_path,
                {
                    "app": app_name,
                    "task_id": parsed.task_id,
                    "batch_id": parsed.batch_id,
                    "task_log": str(resolved_task_path),
                    "verify_exit_code": verify_ret,
                    "recheck_log": str(recheck_result.log_path),
                    "task_archived": True,
                    "single_task_batch_closed": True,
                    "historical_selection_stale_after_close": True,
                    "queue_requires_reresolve_after_close": True,
                    "next_queue_head_after_close": queue_head_after_close,
                    "next_action": next_action,
                },
            )
            print(f"--- tidy-step: step state -> {step_state_path}")
            return 0

        handoff_parts = ["python", "tools/run.py", "tidy-batch", "--app", app_name]
        if workspace.source_scope:
            handoff_parts.extend(["--source-scope", workspace.source_scope])
        if workspace.build_dir_name:
            handoff_parts.extend(["--tidy-build-dir", workspace.build_dir_name])
        handoff_parts.extend(
            clang_tidy_config.build_cli_args(
                config_file=config_file,
                strict_config=strict_config,
            )
        )
        handoff_parts.extend(["--batch-id", parsed.batch_id, "--preset", "sop"])
        handoff_command = " ".join(handoff_parts)
        next_action = (
            f"Task {parsed.batch_id}/task_{parsed.task_id} is closed. If you continue with batch "
            f"handoff, run `{handoff_command}`. After refresh completes, the historical batch/task "
            "selection becomes stale; re-resolve the current smallest pending task from tasks/ "
            "before continuing, and do not reuse historical batch/task ids."
        )
        print(f"--- tidy-step: {next_action}")

        step_state_path = tidy_layout.automation_dir / "tidy_step_last.json"
        tidy_shared.write_json_dict(
            step_state_path,
            {
                "app": app_name,
                "task_id": parsed.task_id,
                "batch_id": parsed.batch_id,
                "task_log": str(resolved_task_path),
                "verify_exit_code": verify_ret,
                "recheck_log": str(recheck_result.log_path),
                "task_archived": True,
                "handoff_batch_id": parsed.batch_id,
                "queue_requires_reresolve_after_batch": True,
                "handoff_command": handoff_command,
                "next_action": next_action,
            },
        )
        print(f"--- tidy-step: step state -> {step_state_path}")
        return 0

    def _sync_queue_snapshot_after_close(
        self,
        *,
        app_name: str,
        tidy_build_dir_name: str,
        source_scope: str | None,
        closed_batch_id: str,
        closed_task_id: str,
        closed_task_path: Path,
        recheck_log_path: Path,
        queue_head_after_close: dict | None,
        config_file: str | None,
        strict_config: bool,
    ) -> None:
        sync_queue_snapshot_after_close(
            self.ctx,
            app_name=app_name,
            tidy_build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
            closed_batch_id=closed_batch_id,
            closed_task_id=closed_task_id,
            closed_task_path=closed_task_path,
            recheck_log_path=recheck_log_path,
            queue_head_after_close=queue_head_after_close,
            config_file=config_file,
            strict_config=strict_config,
        )

    def _build_queue_head(self, tasks_dir: Path) -> dict | None:
        return build_queue_head(tasks_dir)

    def _build_single_task_close_next_action(
        self,
        *,
        batch_id: str,
        task_id: str,
        queue_head: dict | None,
    ) -> str:
        return build_single_task_close_next_action(
            batch_id=batch_id,
            task_id=task_id,
            queue_head=queue_head,
        )

    def _build_close_transition_summary(
        self,
        *,
        batch_id: str,
        task_id: str,
        queue_head: dict | None,
    ) -> str:
        return build_close_transition_summary(
            batch_id=batch_id,
            task_id=task_id,
            queue_head=queue_head,
        )

    def _run_task_recheck(
        self,
        *,
        app_name: str,
        parsed,
        tidy_build_dir_name: str,
        source_scope: str | None,
        config_file: str | None = None,
        strict_config: bool = False,
    ) -> TaskRecheckResult:
        return run_task_recheck(
            self.ctx,
            app_name=app_name,
            parsed=parsed,
            tidy_build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
            config_file=config_file,
            strict_config=strict_config,
            refresh_runner=tidy_refresh_runner,
            run_command_fn=run_command,
            extract_diagnostics_fn=log_parser.extract_diagnostics,
        )

    def _collect_recheck_files(self, parsed) -> list[Path]:
        return collect_recheck_files(parsed)

    def _match_remaining_diagnostics(
        self,
        parsed,
        diagnostics: list[dict],
    ) -> list[dict]:
        return match_remaining_diagnostics(parsed, diagnostics)

    @staticmethod
    def _path_key(path_like) -> str:
        return path_key(path_like)

    @staticmethod
    def _can_continue_after_fix_failures(fix_result) -> bool:
        return can_continue_after_fix_failures(fix_result)

    def _handle_stale_task_before_fix(
        self,
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
    ) -> int | None:
        return handle_stale_task_before_fix(
            self.ctx,
            app_name=app_name,
            task_ctx=task_ctx,
            verify_build_dir_name=verify_build_dir_name,
            profile_name=profile_name,
            concise=concise,
            kill_build_procs=kill_build_procs,
            workspace=workspace,
            resolved_task_path=resolved_task_path,
            config_file=config_file,
            strict_config=strict_config,
            run_task_recheck_fn=self._run_task_recheck,
            build_queue_head_fn=self._build_queue_head,
            sync_queue_snapshot_after_close_fn=self._sync_queue_snapshot_after_close,
            rewrite_task_artifacts_from_recheck_fn=self._rewrite_task_artifacts_from_recheck,
        )

    def _rewrite_task_artifacts_from_recheck(
        self,
        *,
        task_ctx,
        diagnostics: list[dict],
        resolved_task_path: Path,
    ) -> None:
        rewrite_task_artifacts_from_recheck(
            task_ctx=task_ctx,
            diagnostics=diagnostics,
            resolved_task_path=resolved_task_path,
        )
