from pathlib import Path

from ....core.context import Context
from ....services import tidy_state
from ...cmd_quality.verify import VerifyCommand
from .. import tidy_result as tidy_result_summary, workspace as tidy_workspace
from .refresh import TidyRefreshCommand
from ..queue.task_log import list_task_paths, load_task_record


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
        concise: bool = False,
        kill_build_procs: bool = False,
        tidy_only: bool = False,
        dry_run: bool = False,
        config_file: str | None = None,
        strict_config: bool = False,
    ) -> int:
        workspace = tidy_workspace.resolve_workspace(
            self.ctx,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        resolved_build_dir_name = workspace.build_dir_name
        if dry_run and tidy_only:
            raise ValueError("--dry-run and --tidy-only cannot be combined")
        verify_mode = "dry-run" if dry_run else ("skip" if tidy_only else "full")
        tasks_dir = self.ctx.get_tidy_layout(app_name, resolved_build_dir_name).tasks_dir

        if dry_run:
            print("--- tidy-close: previewing final full tidy gate (--dry-run).")
            refresh_ret = TidyRefreshCommand(self.ctx).execute(
                app_name=app_name,
                dry_run=True,
                jobs=jobs,
                keep_going=keep_going,
                source_scope=workspace.source_scope,
                build_dir_name=resolved_build_dir_name,
                config_file=config_file,
                strict_config=strict_config,
                concise=concise,
            )
            self._write_result(
                app_name=app_name,
                build_dir_name=resolved_build_dir_name,
                source_scope=workspace.source_scope,
                verify_mode=verify_mode,
                status="dry_run",
                exit_code=refresh_ret,
                config_file=config_file,
                strict_config=strict_config,
                next_action=(
                    "Run tidy-close without --dry-run for the final-full gate, then "
                    "run the verify gate unless --tidy-only is explicitly intended."
                ),
                final_gate={
                    "final_full_tidy": "previewed" if refresh_ret == 0 else "failed",
                    "verify": "not_run",
                    "queue_empty": "not_evaluated",
                },
            )
            return refresh_ret
        for _round_index in range(1, 2):
            # final_full reuses the caller's bounded jobs/concise settings.
            print("--- tidy-close: running final tidy refresh...")
            refresh_ret = TidyRefreshCommand(self.ctx).execute(
                app_name=app_name,
                jobs=jobs,
                keep_going=keep_going,
                source_scope=workspace.source_scope,
                build_dir_name=resolved_build_dir_name,
                config_file=config_file,
                strict_config=strict_config,
                concise=concise,
            )
            if refresh_ret != 0:
                print("--- tidy-close: stage failed -> tidy-refresh")
                self._write_result(
                    app_name=app_name,
                    build_dir_name=resolved_build_dir_name,
                    source_scope=workspace.source_scope,
                    verify_mode=verify_mode,
                    status="refresh_failed",
                    exit_code=refresh_ret,
                    config_file=config_file,
                    strict_config=strict_config,
                    final_gate={
                        "final_full_tidy": "failed",
                        "verify": "not_run",
                        "queue_empty": "unknown",
                    },
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
                        final_gate={
                            "final_full_tidy": "passed",
                            "verify": "failed",
                            "queue_empty": "unknown",
                        },
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
                    final_gate={
                        "final_full_tidy": "passed",
                        "verify": "skipped" if tidy_only else "passed",
                        "queue_empty": "passed",
                    },
                )

            self._print_pending_tasks(pending_tasks)
            pending_next_action = (
                "Final full tidy produced pending diagnostics. Run tidy-agent in bounded "
                "source-cluster slices, then rerun tidy-close; final gate does not auto-fix tasks."
            )
            self._write_result(
                app_name=app_name,
                build_dir_name=resolved_build_dir_name,
                source_scope=workspace.source_scope,
                verify_mode=verify_mode,
                status="pending_after_final_full",
                exit_code=1,
                config_file=config_file,
                strict_config=strict_config,
                next_action=pending_next_action,
                final_gate={
                    "final_full_tidy": "passed",
                    "verify": "skipped" if tidy_only else "passed",
                    "queue_empty": "failed",
                },
            )
            return 1

        self._write_result(
            app_name=app_name,
            build_dir_name=resolved_build_dir_name,
            source_scope=workspace.source_scope,
            verify_mode=verify_mode,
            status="pending_after_final_full",
            exit_code=1,
            config_file=config_file,
            strict_config=strict_config,
            final_gate={
                "final_full_tidy": "passed",
                "verify": "skipped" if tidy_only else "passed",
            "queue_empty": "failed",
            },
        )
        return 1

    def _list_pending_tasks(self, tasks_dir: Path) -> list[Path]:
        if not tasks_dir.exists():
            return []
        return list_task_paths(tasks_dir)

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
        final_gate: dict,
    ) -> int:
        verify_success: bool | None = None if tidy_only else True
        state_path = tidy_state.update_state(
            ctx=self.ctx,
            app_name=app_name,
            tidy_build_dir_name=tidy_build_dir_name,
            last_verify_success=verify_success,
            last_refresh_ok=True,
            extra_fields={
                "last_tidy_close_ok": True,
                "tidy_close_mode": verify_mode,
                "final_gate": final_gate,
            },
        )
        if tidy_only:
            print("--- tidy-close: completed (final-full + empty tasks, verify skipped).")
        else:
            print("--- tidy-close: completed (final-full + verify + empty tasks).")
        print(f"--- tidy-close: tidy state updated -> {state_path}")
        self._write_result(
            app_name=app_name,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
            verify_mode=verify_mode,
            status="completed_tidy_only" if tidy_only else "completed",
            exit_code=0,
            config_file=config_file,
            strict_config=strict_config,
            final_gate=final_gate,
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
        next_action: str | None = None,
        final_gate: dict | None = None,
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
            next_action=next_action,
            final_gate=final_gate,
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
