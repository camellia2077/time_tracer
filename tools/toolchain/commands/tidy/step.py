from __future__ import annotations

from ...core.context import Context
from ..cmd_quality.verify import VerifyCommand
from ..shared import tidy as tidy_shared
from .batch import TidyBatchCommand
from .task_fix import TidyTaskFixCommand
from .task_log import list_task_paths, parse_task_log, resolve_task_log_path
from .workspace import resolve_workspace


class TidyStepCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        *,
        app_name: str,
        task_log_path: str | None = None,
        batch_id: str | None = None,
        task_id: str | None = None,
        tidy_build_dir_name: str | None = None,
        source_scope: str | None = None,
        verify_build_dir_name: str | None = None,
        profile_name: str | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
        dry_run: bool = False,
        strict: bool = False,
    ) -> int:
        workspace = resolve_workspace(
            self.ctx,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        tidy_layout = self.ctx.get_tidy_layout(app_name, workspace.build_dir_name)
        build_tidy_dir = tidy_layout.root
        tasks_dir = tidy_layout.tasks_dir
        resolved_task_path = resolve_task_log_path(
            tasks_dir,
            task_log_path=task_log_path,
            batch_id=batch_id,
            task_id=task_id,
        )
        parsed = parse_task_log(resolved_task_path)
        print(
            f"--- tidy-step: selected {parsed.batch_id}/task_{parsed.task_id} "
            f"({parsed.source_file or resolved_task_path})"
        )

        fix_ret = TidyTaskFixCommand(self.ctx).execute(
            app_name=app_name,
            task_log_path=str(resolved_task_path),
            tidy_build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            dry_run=dry_run,
            strict=strict,
            report_suffix="step",
        )
        if fix_ret != 0:
            print("--- tidy-step: task auto-fix failed or produced no supported edits.")
            return fix_ret

        if dry_run:
            print("--- tidy-step: dry-run mode, skip verify/batch follow-up.")
            return 0

        print("--- tidy-step: running task-scope verify...")
        verify_ret = VerifyCommand(self.ctx).execute(
            app_name=app_name,
            tidy=False,
            build_dir_name=verify_build_dir_name,
            profile_name=profile_name,
            concise=concise,
            kill_build_procs=kill_build_procs,
            verify_scope="task",
        )
        if verify_ret != 0:
            print("--- tidy-step: task-scope verify failed.")
            return verify_ret

        batch_tasks = list_task_paths(tasks_dir, batch_id=parsed.batch_id)
        if len(batch_tasks) == 1:
            print(
                f"--- tidy-step: {parsed.batch_id} contains a single task. "
                "Running tidy-batch --preset sop..."
            )
            return TidyBatchCommand(self.ctx).execute(
                app_name=app_name,
                batch_id=parsed.batch_id,
                strict_clean=True,
                full_every=3,
                keep_going=True,
                run_verify=True,
                verify_build_dir_name=verify_build_dir_name,
                tidy_build_dir_name=workspace.build_dir_name,
                source_scope=workspace.source_scope,
                profile_name=profile_name,
                concise=concise,
                kill_build_procs=kill_build_procs,
            )

        next_action = (
            "Next: run `python tools/run.py tidy-batch --app "
            f"{app_name}"
        )
        if workspace.source_scope:
            next_action += f" --source-scope {workspace.source_scope}"
        if workspace.build_dir_name:
            next_action += f" --tidy-build-dir {workspace.build_dir_name}"
        next_action += f" --batch-id {parsed.batch_id} --preset sop`"
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
                "next_action": next_action,
            },
        )
        print(f"--- tidy-step: step state -> {step_state_path}")
        return 0
