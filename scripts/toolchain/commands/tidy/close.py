from pathlib import Path

from ...core.context import Context
from ...services import batch_state
from ..cmd_quality.verify import VerifyCommand
from .refresh import TidyRefreshCommand


class TidyCloseCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        keep_going: bool | None = None,
        verify_build_dir_name: str | None = None,
        profile_name: str | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
    ) -> int:
        print("--- tidy-close: running final tidy refresh...")
        refresh_ret = TidyRefreshCommand(self.ctx).execute(
            app_name=app_name,
            final_full=True,
            keep_going=keep_going,
        )
        if refresh_ret != 0:
            print("--- tidy-close: stage failed -> tidy-refresh --final-full")
            return refresh_ret

        print("--- tidy-close: running verify gate...")
        verify_ret = VerifyCommand(self.ctx).execute(
            app_name=app_name,
            tidy=False,
            build_dir_name=verify_build_dir_name,
            profile_name=profile_name,
            concise=concise,
            kill_build_procs=kill_build_procs,
        )
        if verify_ret != 0:
            print("--- tidy-close: stage failed -> verify")
            return verify_ret

        tasks_dir = self.ctx.get_app_dir(app_name) / "build_tidy" / "tasks"
        pending_tasks = self._list_pending_tasks(tasks_dir)
        if pending_tasks:
            print(
                f"--- tidy-close: stage failed -> pending task logs remain ({len(pending_tasks)})."
            )
            for task_path in pending_tasks[:10]:
                print(f"  - {task_path}")
            if len(pending_tasks) > 10:
                print(f"  - ... ({len(pending_tasks) - 10} more)")
            return 1

        state_path = batch_state.update_state(
            ctx=self.ctx,
            app_name=app_name,
            last_verify_success=True,
            last_refresh_ok=True,
            extra_fields={"last_tidy_close_ok": True},
        )
        print("--- tidy-close: completed (final-full + verify + empty tasks).")
        print(f"--- tidy-close: batch state updated -> {state_path}")
        return 0

    def _list_pending_tasks(self, tasks_dir: Path) -> list[Path]:
        if not tasks_dir.exists():
            return []
        return sorted(tasks_dir.rglob("task_*.log"), key=lambda path: str(path))
