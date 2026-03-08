from pathlib import Path

from ...core.context import Context
from ...services import batch_state
from ..cmd_quality.verify import VerifyCommand
from . import tidy_result as tidy_result_summary, workspace as tidy_workspace
from .refresh import TidyRefreshCommand


class TidyCloseCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        keep_going: bool | None = None,
        verify_build_dir_name: str | None = None,
        tidy_build_dir_name: str | None = None,
        source_scope: str | None = None,
        profile_name: str | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
        tidy_only: bool = False,
    ) -> int:
        workspace = tidy_workspace.resolve_workspace(
            self.ctx,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        resolved_build_dir_name = workspace.build_dir_name
        verify_mode = "skip" if tidy_only else "full"
        print("--- tidy-close: running final tidy refresh...")
        refresh_ret = TidyRefreshCommand(self.ctx).execute(
            app_name=app_name,
            final_full=True,
            keep_going=keep_going,
            source_scope=workspace.source_scope,
            build_dir_name=resolved_build_dir_name,
        )
        if refresh_ret != 0:
            print("--- tidy-close: stage failed -> tidy-refresh --final-full")
            tidy_result_summary.write_tidy_result(
                ctx=self.ctx,
                app_name=app_name,
                stage="tidy-close",
                status="refresh_failed",
                exit_code=refresh_ret,
                build_dir_name=resolved_build_dir_name,
                source_scope=workspace.source_scope,
                verify_mode=verify_mode,
            )
            return refresh_ret

        if tidy_only:
            print("--- tidy-close: skip verify gate (--tidy-only).")
        else:
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
                tidy_result_summary.write_tidy_result(
                    ctx=self.ctx,
                    app_name=app_name,
                    stage="tidy-close",
                    status="verify_failed",
                    exit_code=verify_ret,
                    build_dir_name=resolved_build_dir_name,
                    source_scope=workspace.source_scope,
                    verify_mode=verify_mode,
                )
                return verify_ret

        tasks_dir = self.ctx.get_tidy_layout(app_name, resolved_build_dir_name).tasks_dir
        pending_tasks = self._list_pending_tasks(tasks_dir)
        if pending_tasks:
            print(
                f"--- tidy-close: stage failed -> pending task logs remain ({len(pending_tasks)})."
            )
            for task_path in pending_tasks[:10]:
                print(f"  - {task_path}")
            if len(pending_tasks) > 10:
                print(f"  - ... ({len(pending_tasks) - 10} more)")
            tidy_result_summary.write_tidy_result(
                ctx=self.ctx,
                app_name=app_name,
                stage="tidy-close",
                status="pending_tasks",
                exit_code=1,
                build_dir_name=resolved_build_dir_name,
                source_scope=workspace.source_scope,
                verify_mode=verify_mode,
            )
            return 1

        verify_success: bool | None = None if tidy_only else True
        state_path = batch_state.update_state(
            ctx=self.ctx,
            app_name=app_name,
            tidy_build_dir_name=resolved_build_dir_name,
            last_verify_success=verify_success,
            last_refresh_ok=True,
            extra_fields={
                "last_tidy_close_ok": True,
                "tidy_close_mode": verify_mode,
            },
        )
        if tidy_only:
            print("--- tidy-close: completed (final-full + empty tasks, verify skipped).")
        else:
            print("--- tidy-close: completed (final-full + verify + empty tasks).")
        print(f"--- tidy-close: batch state updated -> {state_path}")
        tidy_result_summary.write_tidy_result(
            ctx=self.ctx,
            app_name=app_name,
            stage="tidy-close",
            status="completed_tidy_only" if tidy_only else "completed",
            exit_code=0,
            build_dir_name=resolved_build_dir_name,
            source_scope=workspace.source_scope,
            verify_mode=verify_mode,
        )
        return 0

    def _list_pending_tasks(self, tasks_dir: Path) -> list[Path]:
        if not tasks_dir.exists():
            return []
        return sorted(tasks_dir.rglob("task_*.log"), key=lambda path: str(path))
