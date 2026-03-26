import re
import time
from pathlib import Path

from ...core.context import Context
from ...services import batch_state
from ..cmd_quality.verify import VerifyCommand
from ..shared import tidy as tidy_shared
from . import tidy_result as tidy_result_summary, workspace as tidy_workspace
from .batch_internal.tidy_batch_checkpoint import load_checkpoint, save_checkpoint
from .batch_internal.tidy_batch_pipeline import run_refresh_stage, should_run_stage, timeout_reached
from .clean import CleanCommand
from .refresh import TidyRefreshCommand
from .task_log import list_task_paths, load_task_record

TASK_FILE_PATTERN = re.compile(r"^task_(\d+)\.(?:json|log|toon)$")
_BATCH_STAGES = ("verify", "clean", "refresh", "finalize")


class TidyBatchCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        batch_id: str,
        strict_clean: bool = False,
        full_every: int = 3,
        keep_going: bool | None = None,
        run_verify: bool = False,
        verify_build_dir_name: str | None = None,
        tidy_build_dir_name: str | None = None,
        source_scope: str | None = None,
        profile_name: str | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
        timeout_seconds: int | None = None,
        resume_checkpoint: bool = True,
    ) -> int:
        workspace = tidy_workspace.resolve_workspace(
            self.ctx,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        resolved_build_dir_name = workspace.build_dir_name

        def finalize(code: int, status: str) -> int:
            verify_mode = "full" if run_verify else "skip"
            tidy_result_summary.write_tidy_result(
                ctx=self.ctx,
                app_name=app_name,
                stage="tidy-batch",
                status=status,
                exit_code=code,
                build_dir_name=resolved_build_dir_name,
                source_scope=workspace.source_scope,
                verify_mode=verify_mode,
                historical_batch_id=normalized_batch if 'normalized_batch' in locals() else None,
                historical_task_ids=task_ids if 'task_ids' in locals() else None,
                queue_requires_reresolve=(status == "completed"),
            )
            return code

        start_time = time.monotonic()
        try:
            normalized_batch = self._normalize_batch_name(batch_id)
        except ValueError as exc:
            print(f"--- tidy-batch: {exc}")
            return finalize(2, "invalid_batch_id")

        tidy_layout = self.ctx.get_tidy_layout(app_name, resolved_build_dir_name)
        tasks_dir = tidy_layout.tasks_dir
        tasks_done_dir = tidy_layout.tasks_done_dir
        batch_dir = tasks_dir / normalized_batch
        done_batch_dir = tasks_done_dir / normalized_batch

        task_ids = self._collect_task_ids(batch_dir)
        if not task_ids and not done_batch_dir.exists():
            print(f"--- tidy-batch: no tasks found in {batch_dir}")
            return finalize(1, "missing_batch")

        checkpoint = (
            load_checkpoint(
                ctx=self.ctx,
                app_name=app_name,
                batch_id=normalized_batch,
                stage_order=_BATCH_STAGES,
                tidy_build_dir_name=resolved_build_dir_name,
            )
            if resume_checkpoint
            else None
        )
        next_stage = checkpoint.get("next_stage", "verify") if checkpoint else "verify"
        if checkpoint:
            print(
                f"--- tidy-batch: resume from checkpoint "
                f"(batch={normalized_batch}, next_stage={next_stage})."
            )

        verify_success: bool | None = None
        if run_verify and should_run_stage(next_stage, "verify", _BATCH_STAGES):
            if timeout_reached(start_time, timeout_seconds):
                save_checkpoint(
                    ctx=self.ctx,
                    app_name=app_name,
                    tidy_build_dir_name=resolved_build_dir_name,
                    batch_id=normalized_batch,
                    next_stage="verify",
                    task_ids=task_ids,
                    verify_success=verify_success,
                )
                print("--- tidy-batch: timeout reached before verify stage.")
                return finalize(124, "timeout_before_verify")
            print("--- tidy-batch: running verify gate...")
            verify_ret = VerifyCommand(self.ctx).execute(
                app_name=app_name,
                tidy=False,
                build_dir_name=verify_build_dir_name,
                profile_name=profile_name,
                concise=concise,
                kill_build_procs=kill_build_procs,
            )
            if verify_ret != 0:
                print("--- tidy-batch: verify failed.")
                return finalize(verify_ret, "verify_failed")
            verify_success = True
            save_checkpoint(
                ctx=self.ctx,
                app_name=app_name,
                tidy_build_dir_name=resolved_build_dir_name,
                batch_id=normalized_batch,
                next_stage="clean",
                task_ids=task_ids,
                verify_success=verify_success,
            )

        if task_ids and should_run_stage(next_stage, "clean", _BATCH_STAGES):
            if timeout_reached(start_time, timeout_seconds):
                save_checkpoint(
                    ctx=self.ctx,
                    app_name=app_name,
                    tidy_build_dir_name=resolved_build_dir_name,
                    batch_id=normalized_batch,
                    next_stage="clean",
                    task_ids=task_ids,
                    verify_success=verify_success,
                )
                print("--- tidy-batch: timeout reached before clean stage.")
                return finalize(124, "timeout_before_clean")
            print(f"--- tidy-batch: cleaning {len(task_ids)} task records from {normalized_batch}.")
            clean_ret = CleanCommand(self.ctx).execute(
                app_name=app_name,
                task_ids=task_ids,
                strict=strict_clean,
                batch_id=normalized_batch,
                tidy_build_dir_name=resolved_build_dir_name,
            )
            if clean_ret != 0:
                print("--- tidy-batch: clean failed.")
                return finalize(clean_ret, "clean_failed")
            save_checkpoint(
                ctx=self.ctx,
                app_name=app_name,
                tidy_build_dir_name=resolved_build_dir_name,
                batch_id=normalized_batch,
                next_stage="refresh",
                task_ids=task_ids,
                verify_success=verify_success,
            )
        elif not task_ids:
            print("--- tidy-batch: batch already moved to tasks_done, skip clean stage.")

        if should_run_stage(next_stage, "refresh", _BATCH_STAGES):
            if timeout_reached(start_time, timeout_seconds):
                save_checkpoint(
                    ctx=self.ctx,
                    app_name=app_name,
                    tidy_build_dir_name=resolved_build_dir_name,
                    batch_id=normalized_batch,
                    next_stage="refresh",
                    task_ids=task_ids,
                    verify_success=verify_success,
                )
                print("--- tidy-batch: timeout reached before refresh stage.")
                return finalize(124, "timeout_before_refresh")
            tidy_result_summary.write_tidy_result(
                ctx=self.ctx,
                app_name=app_name,
                stage="tidy-batch",
                status="refresh_running",
                exit_code=0,
                build_dir_name=resolved_build_dir_name,
                source_scope=workspace.source_scope,
                verify_mode="full" if run_verify else "skip",
                next_action=(
                    f"Refresh running for {normalized_batch}. When it completes, treat the "
                    "historical batch/task selection as stale, then re-resolve the current "
                    "queue from tasks/ before continuing. Do not reuse historical batch/task ids."
                ),
                historical_batch_id=normalized_batch,
                historical_task_ids=task_ids,
                queue_requires_reresolve=True,
            )
            print(f"--- tidy-batch: refreshing tidy state for {normalized_batch}...")
            refresh_ret = run_refresh_stage(
                ctx=self.ctx,
                refresh_command_cls=TidyRefreshCommand,
                app_name=app_name,
                batch_id=normalized_batch,
                full_every=full_every,
                keep_going=keep_going,
                source_scope=workspace.source_scope,
                tidy_build_dir_name=resolved_build_dir_name,
                start_time=start_time,
                timeout_seconds=timeout_seconds,
            )
            if refresh_ret == 124:
                save_checkpoint(
                    ctx=self.ctx,
                    app_name=app_name,
                    tidy_build_dir_name=resolved_build_dir_name,
                    batch_id=normalized_batch,
                    next_stage="refresh",
                    task_ids=task_ids,
                    verify_success=verify_success,
                )
                print("--- tidy-batch: timeout reached during refresh stage.")
                return finalize(124, "timeout_during_refresh")
            if refresh_ret != 0:
                print("--- tidy-batch: tidy-refresh failed.")
                return finalize(refresh_ret, "refresh_failed")
            save_checkpoint(
                ctx=self.ctx,
                app_name=app_name,
                tidy_build_dir_name=resolved_build_dir_name,
                batch_id=normalized_batch,
                next_stage="finalize",
                task_ids=task_ids,
                verify_success=verify_success,
            )

        if verify_success is None and strict_clean:
            verify_success = True

        if timeout_reached(start_time, timeout_seconds):
            save_checkpoint(
                ctx=self.ctx,
                app_name=app_name,
                tidy_build_dir_name=resolved_build_dir_name,
                batch_id=normalized_batch,
                next_stage="finalize",
                task_ids=task_ids,
                verify_success=verify_success,
            )
            print("--- tidy-batch: timeout reached before finalize stage.")
            return finalize(124, "timeout_before_finalize")

        queue_head = self._build_queue_head(tasks_dir)
        historical_batch = tidy_result_summary.build_historical_batch_summary(
            tasks_dir=tasks_dir,
            historical_batch_id=normalized_batch,
            historical_task_ids=task_ids,
            queue_head=queue_head,
        )
        next_action = tidy_result_summary.build_queue_reresolve_next_action(
            historical_batch=historical_batch,
            queue_head=queue_head,
        )
        state_path = batch_state.update_state(
            ctx=self.ctx,
            app_name=app_name,
            tidy_build_dir_name=resolved_build_dir_name,
            batch_id=normalized_batch,
            cleaned_task_ids=task_ids,
            last_verify_success=verify_success,
            last_refresh_ok=True,
            extra_fields={
                "last_tidy_batch_ok": True,
                "tidy_batch_checkpoint": None,
                "queue_requires_reresolve": True,
                "historical_selection_stale": True,
                "reparse_required_reason": "refresh_completed",
                "next_queue_head": queue_head,
                "replacement_queue_head": (
                    historical_batch.get("replacement_queue_head") if historical_batch else None
                ),
                "historical_batch": historical_batch,
                "queue_transition_summary": (
                    historical_batch.get("transition_summary") if historical_batch else None
                ),
                "next_action": next_action,
            },
        )
        print(
            f"--- tidy-batch summary: batch={normalized_batch}, "
            f"cleaned={len(task_ids)}, full_every={full_every}"
        )
        print(f"--- tidy-batch: batch state updated -> {state_path}")
        return finalize(0, "completed")

    def _collect_task_ids(self, batch_dir: Path) -> list[str]:
        if not batch_dir.exists() or not batch_dir.is_dir():
            return []
        task_ids: list[str] = []
        for task_file in list_task_paths(batch_dir.parent, batch_id=batch_dir.name):
            match = TASK_FILE_PATTERN.match(task_file.name)
            if not match:
                continue
            task_ids.append(match.group(1).zfill(3))
        return task_ids

    def _normalize_batch_name(self, batch_id: str) -> str:
        return tidy_shared.normalize_required_batch_name(batch_id)

    def _build_queue_head(self, tasks_dir: Path) -> dict | None:
        pending_logs = list_task_paths(tasks_dir)
        if not pending_logs:
            return None
        head_path = pending_logs[0]
        parsed = load_task_record(head_path)
        return {
            "task_id": parsed.task_id,
            "batch_id": parsed.batch_id,
            "queue_batch_id": parsed.batch_id,
            "source_file": parsed.source_file or str(head_path),
            "task_log": str(head_path),
            "checks": list(parsed.checks),
        }
