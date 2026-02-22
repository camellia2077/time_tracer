import re
from pathlib import Path

from ...core.context import Context
from ...services import batch_state
from ..cmd_quality.verify import VerifyCommand
from ..shared import tidy as tidy_shared
from .clean import CleanCommand
from .refresh import TidyRefreshCommand

TASK_FILE_PATTERN = re.compile(r"^task_(\d+)\.log$")


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
    ) -> int:
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

        verify_success: bool | None = None
        if run_verify:
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

        if task_ids:
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
        else:
            print("--- tidy-batch: batch already moved to tasks_done, skip clean stage.")

        print(f"--- tidy-batch: refreshing tidy state for {normalized_batch}...")
        refresh_ret = TidyRefreshCommand(self.ctx).execute(
            app_name=app_name,
            batch_id=normalized_batch,
            full_every=full_every,
            keep_going=keep_going,
        )
        if refresh_ret != 0:
            print("--- tidy-batch: tidy-refresh failed.")
            return refresh_ret

        if verify_success is None and strict_clean:
            verify_success = True

        state_path = batch_state.update_state(
            ctx=self.ctx,
            app_name=app_name,
            batch_id=normalized_batch,
            cleaned_task_ids=task_ids,
            last_verify_success=verify_success,
            last_refresh_ok=True,
            extra_fields={"last_tidy_batch_ok": True},
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
