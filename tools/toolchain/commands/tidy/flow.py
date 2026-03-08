from ...core.context import Context
from . import tidy_result as tidy_result_summary, workspace as tidy_workspace
from .flow_internal import flow_runner as tidy_flow_runner


class TidyFlowCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        process_all: bool = False,
        n: int = 1,
        resume: bool = False,
        test_every: int = 3,
        concise: bool = False,
        jobs: int | None = None,
        parse_workers: int | None = None,
        keep_going: bool | None = None,
        run_tidy_fix: bool | None = None,
        tidy_fix_limit: int | None = None,
        build_dir_name: str | None = None,
        tidy_build_dir_name: str | None = None,
        source_scope: str | None = None,
        profile_name: str | None = None,
        kill_build_procs: bool = False,
    ) -> int:
        workspace = tidy_workspace.resolve_workspace(
            self.ctx,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        options = tidy_flow_runner.TidyFlowOptions(
            app_name=app_name,
            process_all=process_all,
            n=n,
            resume=resume,
            test_every=test_every,
            concise=concise,
            jobs=jobs,
            parse_workers=parse_workers,
            keep_going=keep_going,
            run_tidy_fix=run_tidy_fix,
            tidy_fix_limit=tidy_fix_limit,
            build_dir_name=build_dir_name,
            tidy_build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            profile_name=profile_name,
            kill_build_procs=kill_build_procs,
        )
        ret = tidy_flow_runner.execute_flow(self.ctx, options)
        if ret == 0:
            status = "completed"
        elif ret == 2:
            status = "manual_loop_required"
        else:
            status = "failed"
        tidy_result_summary.write_tidy_result(
            ctx=self.ctx,
            app_name=app_name,
            stage="tidy-flow",
            status=status,
            exit_code=ret,
            build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            verify_mode="full",
        )
        return ret
