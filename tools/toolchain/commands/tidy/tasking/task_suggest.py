from __future__ import annotations

from ....core.context import Context
from .task_auto_fix import suggest_task_refactors, write_task_suggestion_report
from .task_context import resolve_task_context
from ..workspace import resolve_workspace


class TidyTaskSuggestCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        *,
        task_log_path: str,
    ) -> int:
        task_ctx = resolve_task_context(self.ctx, task_log_path=task_log_path)
        workspace = resolve_workspace(
            self.ctx,
            build_dir_name=task_ctx.tidy_build_dir_name,
            source_scope=task_ctx.source_scope,
        )
        tidy_layout = self.ctx.get_tidy_layout(task_ctx.app_name, workspace.build_dir_name)
        build_tidy_dir = tidy_layout.root
        task_path = task_ctx.task_json_path
        parsed = task_ctx.parsed_task
        suggestions = suggest_task_refactors(parsed)
        json_path, markdown_path = write_task_suggestion_report(
            build_tidy_dir=build_tidy_dir,
            app_name=task_ctx.app_name,
            parsed=parsed,
            task_path=task_path,
            workspace_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            suggestions=suggestions,
        )
        print(
            f"--- tidy-task-suggest: task={parsed.task_id}, suggestions={len(suggestions)}"
        )
        print(f"--- tidy-task-suggest: report json -> {json_path}")
        print(f"--- tidy-task-suggest: report md   -> {markdown_path}")
        return 0
