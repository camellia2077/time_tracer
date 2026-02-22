from ...core.context import Context
from ..tidy import TidyCommand
from . import flow_stages as tidy_flow_stages, flow_state as tidy_flow_state
from .fix import TidyFixCommand


def run_prepare_phase(
    ctx: Context,
    options,
    state: dict,
    tasks_dir,
    effective_run_tidy_fix: bool,
    effective_tidy_fix_limit: int,
    effective_keep_going: bool,
) -> int:
    tidy_flow_state.set_phase(state, "prepare_tasks")
    task_log_exists = tidy_flow_stages.has_task_logs(tasks_dir)
    if options.resume and task_log_exists:
        print(
            "--- tidy-flow: resume enabled and existing task logs found, "
            "skip tidy-fix/tidy generation."
        )
        tidy_flow_state.set_step(state, "prepare_tasks", "skipped")
        return 0

    if effective_run_tidy_fix:
        print("--- tidy-flow: running pre-pass tidy-fix...")
        tidy_fix_ret = TidyFixCommand(ctx).execute(
            app_name=options.app_name,
            limit=effective_tidy_fix_limit,
            jobs=options.jobs,
            keep_going=effective_keep_going,
        )
        state["tidy_fix_exit_code"] = tidy_fix_ret
        if tidy_fix_ret != 0:
            print(
                "--- tidy-flow: tidy-fix returned non-zero; continue to tidy task generation."
            )

    print("--- tidy-flow: generating/refreshing tidy tasks...")
    tidy_ret = TidyCommand(ctx).execute(
        app_name=options.app_name,
        extra_args=[],
        jobs=options.jobs,
        parse_workers=options.parse_workers,
        keep_going=effective_keep_going,
    )
    if tidy_ret != 0:
        tidy_flow_state.set_step(state, "prepare_tasks", "failed", tidy_ret)
        return tidy_ret

    tidy_flow_state.set_step(state, "prepare_tasks", "done", 0)
    return 0
