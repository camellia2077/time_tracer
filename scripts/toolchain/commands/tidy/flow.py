from ...core.context import Context
from . import flow_runner as tidy_flow_runner


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
        profile_name: str | None = None,
        kill_build_procs: bool = False,
    ) -> int:
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
            profile_name=profile_name,
            kill_build_procs=kill_build_procs,
        )
        return tidy_flow_runner.execute_flow(self.ctx, options)
