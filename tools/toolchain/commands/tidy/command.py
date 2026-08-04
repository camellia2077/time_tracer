from ...core.context import Context
from . import (
    command_execute as tidy_command_execute,
    tidy_result as tidy_result_summary,
    workspace as tidy_workspace,
)


class TidyCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    # --- Public entrypoints ---
    def execute(
        self,
        app_name: str,
        extra_args: list[str] | None = None,
        jobs: int | None = None,
        concise: bool = False,
        profile_name: str | None = None,
        kill_build_procs: bool = False,
        keep_going: bool | None = None,
        source_scope: str | None = None,
        build_dir_name: str | None = None,
        task_view: str | None = None,
        config_file: str | None = None,
        strict_config: bool = False,
    ) -> int:
        workspace = tidy_workspace.resolve_workspace(
            self.ctx,
            build_dir_name=build_dir_name,
            source_scope=source_scope,
        )
        ret = tidy_command_execute.execute_tidy_command(
            ctx=self.ctx,
            app_name=app_name,
            extra_args=extra_args,
            jobs=jobs,
            concise=concise,
            profile_name=profile_name,
            kill_build_procs=kill_build_procs,
            keep_going=keep_going,
            source_scope=workspace.source_scope,
            build_dir_name=workspace.build_dir_name,
            task_view=task_view,
            prebuild_targets=workspace.prebuild_targets,
            config_file=config_file,
            strict_config=strict_config,
        )
        status = "completed" if ret == 0 else "failed"
        tidy_result_summary.write_tidy_result(
            ctx=self.ctx,
            app_name=app_name,
            stage="tidy",
            status=status,
            exit_code=ret,
            build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            verify_mode="skip",
            config_file=config_file,
            strict_config=strict_config,
        )
        return ret
