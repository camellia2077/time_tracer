from __future__ import annotations

from ....core.context import Context
from .task_auto_fix import run_task_auto_fix


class TidyTaskPatchCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        *,
        task_log_path: str,
        strict: bool = False,
    ) -> int:
        result = run_task_auto_fix(
            self.ctx,
            task_log_path=task_log_path,
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
