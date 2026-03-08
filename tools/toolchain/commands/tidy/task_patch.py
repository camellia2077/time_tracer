from __future__ import annotations

from ...core.context import Context
from .task_auto_fix import run_task_auto_fix


class TidyTaskPatchCommand:
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
        strict: bool = False,
    ) -> int:
        result = run_task_auto_fix(
            self.ctx,
            app_name=app_name,
            task_log_path=task_log_path,
            batch_id=batch_id,
            task_id=task_id,
            tidy_build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
            dry_run=True,
            report_suffix="patch",
        )
        print(
            f"--- tidy-task-patch: task={result.task_id}, previewed={result.previewed}, "
            f"skipped={result.skipped}, failed={result.failed}"
        )
        print(f"--- tidy-task-patch: report json -> {result.json_path}")
        print(f"--- tidy-task-patch: report md   -> {result.markdown_path}")
        return result.exit_code(strict=strict)
