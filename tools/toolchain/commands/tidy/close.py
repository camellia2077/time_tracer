from pathlib import Path

from ...core.context import Context
from ...services import batch_state
from ..cmd_quality.verify import VerifyCommand
from . import tidy_result as tidy_result_summary, workspace as tidy_workspace
from .refresh import TidyRefreshCommand
from .step import TidyStepCommand
from .task_log import list_task_paths, load_task_record


_MAX_STABILIZE_ROUNDS = 8
_MAX_STABILIZE_TASKS_PER_CLOSE = 128


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
        jobs: int | None = None,
        parse_workers: int | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
        tidy_only: bool = False,
        config_file: str | None = None,
        strict_config: bool = False,
        stabilize: bool = False,
    ) -> int:
        workspace = tidy_workspace.resolve_workspace(
            self.ctx,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        resolved_build_dir_name = workspace.build_dir_name
        verify_mode = "skip" if tidy_only else "full"
        tasks_dir = self.ctx.get_tidy_layout(app_name, resolved_build_dir_name).tasks_dir
        max_rounds = _MAX_STABILIZE_ROUNDS if stabilize else 1
        for round_index in range(1, max_rounds + 1):
            # final_full must reuse the caller's bounded jobs/concise settings;
            # otherwise tidy-close --stabilize can look hung while a noisy
            # unbounded full tidy keeps chewing through long-tail translation
            # units after queue draining has already succeeded once.
            print("--- tidy-close: running final tidy refresh...")
            refresh_ret = TidyRefreshCommand(self.ctx).execute(
                app_name=app_name,
                final_full=True,
                jobs=jobs,
                parse_workers=parse_workers,
                keep_going=keep_going,
                source_scope=workspace.source_scope,
                build_dir_name=resolved_build_dir_name,
                config_file=config_file,
                strict_config=strict_config,
                concise=concise,
            )
            if refresh_ret != 0:
                print("--- tidy-close: stage failed -> tidy-refresh --final-full")
                self._write_result(
                    app_name=app_name,
                    build_dir_name=resolved_build_dir_name,
                    source_scope=workspace.source_scope,
                    verify_mode=verify_mode,
                    status="refresh_failed",
                    exit_code=refresh_ret,
                    config_file=config_file,
                    strict_config=strict_config,
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
                    self._write_result(
                        app_name=app_name,
                        build_dir_name=resolved_build_dir_name,
                        source_scope=workspace.source_scope,
                        verify_mode=verify_mode,
                        status="verify_failed",
                        exit_code=verify_ret,
                        config_file=config_file,
                        strict_config=strict_config,
                    )
                    return verify_ret

            pending_tasks = self._list_pending_tasks(tasks_dir)
            if not pending_tasks:
                return self._finish_success(
                    app_name=app_name,
                    tidy_build_dir_name=resolved_build_dir_name,
                    source_scope=workspace.source_scope,
                    verify_mode=verify_mode,
                    tidy_only=tidy_only,
                    config_file=config_file,
                    strict_config=strict_config,
                )

            # This is not old tasks being restored incorrectly. Historically,
            # tidy-close only did one round in close.py:
            # 1. run final_full
            # 2. run verify
            # 3. fail immediately if tasks/ was repopulated
            # A final full tidy can legitimately discover a new wave of task
            # files, so --stabilize keeps draining the refreshed queue and then
            # reruns final_full until a later round stays empty.
            #
            # We intentionally do not reuse tidy-batch here. final_full creates
            # a fresh queue snapshot whose batch folders are only queue slices,
            # not historical identities that are safe to clean wholesale. Using
            # tidy-batch would clean sibling task records that were never fixed,
            # and final_full would simply regenerate them on the next round.
            if not stabilize:
                self._print_pending_tasks(pending_tasks)
                self._write_result(
                    app_name=app_name,
                    build_dir_name=resolved_build_dir_name,
                    source_scope=workspace.source_scope,
                    verify_mode=verify_mode,
                    status="pending_tasks",
                    exit_code=1,
                    config_file=config_file,
                    strict_config=strict_config,
                )
                return 1

            print(
                "--- tidy-close: final full tidy repopulated tasks/, "
                f"draining refreshed queue (round {round_index}/{max_rounds})."
            )
            stabilize_ret, stabilize_status = self._drain_refreshed_queue(
                tasks_dir=tasks_dir,
                verify_build_dir_name=verify_build_dir_name,
                profile_name=profile_name,
                concise=concise,
                kill_build_procs=kill_build_procs,
                config_file=config_file,
                strict_config=strict_config,
            )
            if stabilize_ret != 0:
                self._write_result(
                    app_name=app_name,
                    build_dir_name=resolved_build_dir_name,
                    source_scope=workspace.source_scope,
                    verify_mode=verify_mode,
                    status=stabilize_status,
                    exit_code=stabilize_ret,
                    config_file=config_file,
                    strict_config=strict_config,
                )
                return stabilize_ret

        print(
            "--- tidy-close: stage failed -> stabilize mode exhausted its "
            "final-full retry budget."
        )
        self._write_result(
            app_name=app_name,
            build_dir_name=resolved_build_dir_name,
            source_scope=workspace.source_scope,
            verify_mode=verify_mode,
            status="stabilize_limit_reached",
            exit_code=1,
            config_file=config_file,
            strict_config=strict_config,
        )
        return 1

    def _list_pending_tasks(self, tasks_dir: Path) -> list[Path]:
        if not tasks_dir.exists():
            return []
        return list_task_paths(tasks_dir)

    def _drain_refreshed_queue(
        self,
        *,
        tasks_dir: Path,
        verify_build_dir_name: str | None,
        profile_name: str | None,
        concise: bool,
        kill_build_procs: bool,
        config_file: str | None,
        strict_config: bool,
    ) -> tuple[int, str]:
        processed_tasks = 0
        while True:
            pending_tasks = self._list_pending_tasks(tasks_dir)
            if not pending_tasks:
                print("--- tidy-close: refreshed queue drained, re-running final tidy refresh.")
                return 0, "stabilized"
            if processed_tasks >= _MAX_STABILIZE_TASKS_PER_CLOSE:
                print(
                    "--- tidy-close: stage failed -> stabilize mode hit the "
                    "per-close task budget."
                )
                return 1, "stabilize_task_limit_reached"

            snapshot_before = tuple(str(path) for path in pending_tasks)
            head_path = pending_tasks[0]
            parsed = load_task_record(head_path)
            print(
                "--- tidy-close: stabilize processing "
                f"{parsed.batch_id}/task_{parsed.task_id} ({parsed.source_file or head_path})."
            )
            step_ret = TidyStepCommand(self.ctx).execute(
                task_log_path=str(head_path),
                verify_build_dir_name=verify_build_dir_name,
                profile_name=profile_name,
                concise=concise,
                kill_build_procs=kill_build_procs,
                config_file=config_file,
                strict_config=strict_config,
            )
            if step_ret != 0:
                print("--- tidy-close: stage failed -> stabilize tidy-step")
                return step_ret, "stabilize_step_failed"

            processed_tasks += 1
            snapshot_after = tuple(str(path) for path in self._list_pending_tasks(tasks_dir))
            if snapshot_after == snapshot_before:
                print(
                    "--- tidy-close: stage failed -> stabilize mode made no "
                    "queue progress after closing the current queue head."
                )
                return 1, "stabilize_no_progress"

    def _finish_success(
        self,
        *,
        app_name: str,
        tidy_build_dir_name: str,
        source_scope: str | None,
        verify_mode: str,
        tidy_only: bool,
        config_file: str | None,
        strict_config: bool,
    ) -> int:
        verify_success: bool | None = None if tidy_only else True
        state_path = batch_state.update_state(
            ctx=self.ctx,
            app_name=app_name,
            tidy_build_dir_name=tidy_build_dir_name,
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
        self._write_result(
            app_name=app_name,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
            verify_mode=verify_mode,
            status="completed_tidy_only" if tidy_only else "completed",
            exit_code=0,
            config_file=config_file,
            strict_config=strict_config,
        )
        return 0

    def _write_result(
        self,
        *,
        app_name: str,
        build_dir_name: str,
        source_scope: str | None,
        verify_mode: str,
        status: str,
        exit_code: int,
        config_file: str | None,
        strict_config: bool,
    ) -> None:
        tidy_result_summary.write_tidy_result(
            ctx=self.ctx,
            app_name=app_name,
            stage="tidy-close",
            status=status,
            exit_code=exit_code,
            build_dir_name=build_dir_name,
            source_scope=source_scope,
            verify_mode=verify_mode,
            config_file=config_file,
            strict_config=strict_config,
        )

    @staticmethod
    def _print_pending_tasks(pending_tasks: list[Path]) -> None:
        print(
            f"--- tidy-close: stage failed -> pending task records remain ({len(pending_tasks)})."
        )
        for task_path in pending_tasks[:10]:
            print(f"  - {task_path}")
        if len(pending_tasks) > 10:
            print(f"  - ... ({len(pending_tasks) - 10} more)")
