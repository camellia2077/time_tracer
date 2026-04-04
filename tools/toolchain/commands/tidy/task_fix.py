from __future__ import annotations

from ...core.context import Context
from .task_auto_fix import run_task_auto_fix


class TidyTaskFixCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        *,
        task_log_path: str,
        dry_run: bool = False,
        strict: bool = False,
        report_suffix: str = "fix",
    ) -> int:
        result = run_task_auto_fix(
            self.ctx,
            task_log_path=task_log_path,
            dry_run=dry_run,
            report_suffix=report_suffix,
        )
        print(
            f"--- tidy-task-fix: task={result.task_id}, applied={result.applied}, "
            f"previewed={result.previewed}, skipped={result.skipped}, failed={result.failed}"
        )
        print(f"--- tidy-task-fix: report json -> {result.json_path}")
        print(f"--- tidy-task-fix: report md   -> {result.markdown_path}")
        return result.exit_code(strict=strict)
