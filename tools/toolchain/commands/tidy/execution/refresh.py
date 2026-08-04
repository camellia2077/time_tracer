from .. import tidy_result as tidy_result_summary, workspace as tidy_workspace
from ..command import TidyCommand


class TidyRefreshCommand:
    """Regenerate the current scan -> clusters -> tasks queue."""

    def __init__(self, ctx):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        source_scope: str | None = None,
        build_dir_name: str | None = None,
        task_view: str | None = None,
        dry_run: bool = False,
        jobs: int | None = None,
        keep_going: bool | None = None,
        concise: bool = False,
        config_file: str | None = None,
        strict_config: bool = False,
    ) -> int:
        workspace = tidy_workspace.resolve_workspace(
            self.ctx,
            build_dir_name=build_dir_name,
            source_scope=source_scope,
        )
        tasks_dir = self.ctx.get_tidy_layout(
            app_name,
            workspace.build_dir_name,
        ).tasks_dir
        if dry_run:
            print(
                "--- tidy-refresh: dry-run; would regenerate the current "
                f"scan -> clusters -> tasks queue in {tasks_dir}."
            )
            tidy_result_summary.write_tidy_result(
                ctx=self.ctx,
                app_name=app_name,
                stage="tidy-refresh-dry-run",
                status="previewed",
                exit_code=0,
                build_dir_name=workspace.build_dir_name,
                source_scope=workspace.source_scope,
                verify_mode="skip",
                config_file=config_file,
                strict_config=strict_config,
            )
            return 0

        ret = TidyCommand(self.ctx).execute(
            app_name=app_name,
            jobs=jobs,
            concise=concise,
            keep_going=keep_going,
            source_scope=workspace.source_scope,
            build_dir_name=workspace.build_dir_name,
            task_view=task_view,
            config_file=config_file,
            strict_config=strict_config,
        )
        tidy_result_summary.write_tidy_result(
            ctx=self.ctx,
            app_name=app_name,
            stage="tidy-refresh",
            status="completed" if ret == 0 else "failed",
            exit_code=ret,
            build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            verify_mode="skip",
            config_file=config_file,
            strict_config=strict_config,
        )
        return ret
