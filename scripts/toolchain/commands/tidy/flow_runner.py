from dataclasses import dataclass

from ...core.context import Context
from . import flow_execute as tidy_flow_execute


@dataclass(slots=True)
class TidyFlowOptions:
    app_name: str
    process_all: bool = False
    n: int = 1
    resume: bool = False
    test_every: int = 3
    concise: bool = False
    jobs: int | None = None
    parse_workers: int | None = None
    keep_going: bool | None = None
    run_tidy_fix: bool | None = None
    tidy_fix_limit: int | None = None
    build_dir_name: str | None = None
    profile_name: str | None = None
    kill_build_procs: bool = False


def execute_flow(ctx: Context, options: TidyFlowOptions) -> int:
    return tidy_flow_execute.execute_flow_impl(ctx, options)
