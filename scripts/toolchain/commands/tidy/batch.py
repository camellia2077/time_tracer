import re
import subprocess
import sys
import time
from pathlib import Path

from ...core.context import Context
from ...services import batch_state
from ..cmd_quality.verify import VerifyCommand
from ..shared import tidy as tidy_shared
from .clean import CleanCommand
from .refresh import TidyRefreshCommand

TASK_FILE_PATTERN = re.compile(r"^task_(\d+)\.log$")
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
        profile_name: str | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
        timeout_seconds: int | None = None,
        resume_checkpoint: bool = True,
    ) -> int:
        start_time = time.monotonic()
        try:
            normalized_batch = self._normalize_batch_name(batch_id)
        except ValueError as exc:
            print(f"--- tidy-batch: {exc}")
            return 2

        app_dir = self.ctx.get_app_dir(app_name)
        tasks_dir = app_dir / "build_tidy" / "tasks"
        tasks_done_dir = app_dir / "build_tidy" / "tasks_done"
        batch_dir = tasks_dir / normalized_batch
        done_batch_dir = tasks_done_dir / normalized_batch

        task_ids = self._collect_task_ids(batch_dir)
        if not task_ids and not done_batch_dir.exists():
            print(f"--- tidy-batch: no tasks found in {batch_dir}")
            return 1

        checkpoint = (
            self._load_checkpoint(app_name=app_name, batch_id=normalized_batch)
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
        if run_verify and self._should_run_stage(next_stage, "verify"):
            if self._timeout_reached(start_time, timeout_seconds):
                self._save_checkpoint(
                    app_name=app_name,
                    batch_id=normalized_batch,
                    next_stage="verify",
                    task_ids=task_ids,
                    verify_success=verify_success,
                )
                print("--- tidy-batch: timeout reached before verify stage.")
                return 124
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
                return verify_ret
            verify_success = True
            self._save_checkpoint(
                app_name=app_name,
                batch_id=normalized_batch,
                next_stage="clean",
                task_ids=task_ids,
                verify_success=verify_success,
            )

        if task_ids and self._should_run_stage(next_stage, "clean"):
            if self._timeout_reached(start_time, timeout_seconds):
                self._save_checkpoint(
                    app_name=app_name,
                    batch_id=normalized_batch,
                    next_stage="clean",
                    task_ids=task_ids,
                    verify_success=verify_success,
                )
                print("--- tidy-batch: timeout reached before clean stage.")
                return 124
            print(f"--- tidy-batch: cleaning {len(task_ids)} task logs from {normalized_batch}.")
            clean_ret = CleanCommand(self.ctx).execute(
                app_name=app_name,
                task_ids=task_ids,
                strict=strict_clean,
                batch_id=normalized_batch,
            )
            if clean_ret != 0:
                print("--- tidy-batch: clean failed.")
                return clean_ret
            self._save_checkpoint(
                app_name=app_name,
                batch_id=normalized_batch,
                next_stage="refresh",
                task_ids=task_ids,
                verify_success=verify_success,
            )
        elif not task_ids:
            print("--- tidy-batch: batch already moved to tasks_done, skip clean stage.")

        if self._should_run_stage(next_stage, "refresh"):
            if self._timeout_reached(start_time, timeout_seconds):
                self._save_checkpoint(
                    app_name=app_name,
                    batch_id=normalized_batch,
                    next_stage="refresh",
                    task_ids=task_ids,
                    verify_success=verify_success,
                )
                print("--- tidy-batch: timeout reached before refresh stage.")
                return 124
            print(f"--- tidy-batch: refreshing tidy state for {normalized_batch}...")
            refresh_ret = self._run_refresh_stage(
                app_name=app_name,
                batch_id=normalized_batch,
                full_every=full_every,
                keep_going=keep_going,
                start_time=start_time,
                timeout_seconds=timeout_seconds,
            )
            if refresh_ret == 124:
                self._save_checkpoint(
                    app_name=app_name,
                    batch_id=normalized_batch,
                    next_stage="refresh",
                    task_ids=task_ids,
                    verify_success=verify_success,
                )
                print("--- tidy-batch: timeout reached during refresh stage.")
                return 124
            if refresh_ret != 0:
                print("--- tidy-batch: tidy-refresh failed.")
                return refresh_ret
            self._save_checkpoint(
                app_name=app_name,
                batch_id=normalized_batch,
                next_stage="finalize",
                task_ids=task_ids,
                verify_success=verify_success,
            )

        if verify_success is None and strict_clean:
            verify_success = True

        if self._timeout_reached(start_time, timeout_seconds):
            self._save_checkpoint(
                app_name=app_name,
                batch_id=normalized_batch,
                next_stage="finalize",
                task_ids=task_ids,
                verify_success=verify_success,
            )
            print("--- tidy-batch: timeout reached before finalize stage.")
            return 124

        state_path = batch_state.update_state(
            ctx=self.ctx,
            app_name=app_name,
            batch_id=normalized_batch,
            cleaned_task_ids=task_ids,
            last_verify_success=verify_success,
            last_refresh_ok=True,
            extra_fields={
                "last_tidy_batch_ok": True,
                "tidy_batch_checkpoint": None,
            },
        )
        print(
            f"--- tidy-batch summary: batch={normalized_batch}, "
            f"cleaned={len(task_ids)}, full_every={full_every}"
        )
        print(f"--- tidy-batch: batch state updated -> {state_path}")
        return 0

    def _collect_task_ids(self, batch_dir: Path) -> list[str]:
        if not batch_dir.exists() or not batch_dir.is_dir():
            return []
        task_ids: list[str] = []
        for task_file in sorted(batch_dir.glob("task_*.log"), key=lambda p: p.name):
            match = TASK_FILE_PATTERN.match(task_file.name)
            if not match:
                continue
            task_ids.append(match.group(1).zfill(3))
        return task_ids

    def _normalize_batch_name(self, batch_id: str) -> str:
        return tidy_shared.normalize_required_batch_name(batch_id)

    def _should_run_stage(self, next_stage: str, stage: str) -> bool:
        try:
            return _BATCH_STAGES.index(stage) >= _BATCH_STAGES.index(next_stage)
        except ValueError:
            return True

    def _timeout_reached(self, start_time: float, timeout_seconds: int | None) -> bool:
        if timeout_seconds is None or timeout_seconds <= 0:
            return False
        return (time.monotonic() - start_time) >= timeout_seconds

    def _load_checkpoint(self, app_name: str, batch_id: str) -> dict | None:
        state_path = batch_state.state_path(self.ctx, app_name)
        state = batch_state.load_state(state_path, app_name)
        checkpoint = state.get("tidy_batch_checkpoint")
        if not isinstance(checkpoint, dict):
            return None
        if checkpoint.get("batch_id") != batch_id:
            return None
        if checkpoint.get("next_stage") not in _BATCH_STAGES:
            return None
        return checkpoint

    def _run_refresh_stage(
        self,
        *,
        app_name: str,
        batch_id: str,
        full_every: int,
        keep_going: bool | None,
        start_time: float,
        timeout_seconds: int | None,
    ) -> int:
        if timeout_seconds is None or timeout_seconds <= 0:
            return TidyRefreshCommand(self.ctx).execute(
                app_name=app_name,
                batch_id=batch_id,
                full_every=full_every,
                keep_going=keep_going,
            )

        remaining_seconds = timeout_seconds - (time.monotonic() - start_time)
        if remaining_seconds <= 0:
            return 124

        cmd = [
            sys.executable,
            str(self.ctx.repo_root / "scripts" / "run.py"),
            "tidy-refresh",
            "--app",
            app_name,
            "--batch-id",
            batch_id,
            "--full-every",
            str(full_every),
        ]
        if keep_going is True:
            cmd.append("--keep-going")
        elif keep_going is False:
            cmd.append("--no-keep-going")

        try:
            completed = subprocess.run(
                cmd,
                cwd=self.ctx.repo_root,
                env=self.ctx.setup_env(),
                check=False,
                timeout=max(1, int(remaining_seconds)),
            )
            return int(completed.returncode)
        except subprocess.TimeoutExpired:
            print("--- tidy-batch: refresh stage timeout reached.")
            return 124

    def _save_checkpoint(
        self,
        *,
        app_name: str,
        batch_id: str,
        next_stage: str,
        task_ids: list[str],
        verify_success: bool | None,
    ) -> None:
        checkpoint = {
            "batch_id": batch_id,
            "next_stage": next_stage,
            "task_ids": task_ids,
        }
        batch_state.update_state(
            ctx=self.ctx,
            app_name=app_name,
            batch_id=batch_id,
            cleaned_task_ids=[],
            last_verify_success=verify_success,
            extra_fields={"tidy_batch_checkpoint": checkpoint},
        )
