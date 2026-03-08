from __future__ import annotations

from ...core.context import Context
from .task_auto_fix import suggest_task_refactors, write_task_suggestion_report
from .task_log import parse_task_log, resolve_task_log_path
from .workspace import resolve_workspace


class TidyTaskSuggestCommand:
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
    ) -> int:
        workspace = resolve_workspace(
            self.ctx,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        app_dir = self.ctx.get_app_dir(app_name)
        build_tidy_dir = app_dir / workspace.build_dir_name
        tasks_dir = build_tidy_dir / "tasks"
        task_path = resolve_task_log_path(
            tasks_dir,
            task_log_path=task_log_path,
            batch_id=batch_id,
            task_id=task_id,
        )
        parsed = parse_task_log(task_path)
        suggestions = suggest_task_refactors(parsed)
        json_path, markdown_path = write_task_suggestion_report(
            build_tidy_dir=build_tidy_dir,
            app_name=app_name,
            parsed=parsed,
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
