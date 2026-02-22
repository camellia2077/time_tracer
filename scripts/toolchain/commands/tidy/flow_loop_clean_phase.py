from ...core.context import Context
from . import flow_stages as tidy_flow_stages, flow_state as tidy_flow_state
from .loop import TidyLoopCommand


def run_loop_clean_phase(
    ctx: Context,
    options,
    state: dict,
    tasks_dir,
    effective_n: int,
    effective_test_every: int,
) -> dict:
    tidy_flow_state.set_phase(state, "loop")
    loop_ret = TidyLoopCommand(ctx).execute(
        app_name=options.app_name,
        n=effective_n,
        process_all=options.process_all,
        test_every=effective_test_every,
        concise=options.concise,
        kill_build_procs=options.kill_build_procs,
    )
    if loop_ret not in (0, 2):
        tidy_flow_state.set_step(state, "loop", "failed", loop_ret)
        return {
            "phase_error": loop_ret,
            "final_exit_code": loop_ret,
            "pending_task_ids": [],
            "blocked_task_id": None,
        }

    tidy_flow_state.set_step(state, "loop", "done", loop_ret)

    tidy_flow_state.set_phase(state, "clean")
    cleaned_empty_task_ids = tidy_flow_stages.clean_empty_tasks(
        ctx=ctx,
        app_name=options.app_name,
        tasks_dir=tasks_dir,
    )
    if cleaned_empty_task_ids:
        print(f"--- tidy-flow: cleaned empty task logs: {', '.join(cleaned_empty_task_ids)}")
    state["cleaned_empty_task_ids"] = cleaned_empty_task_ids
    tidy_flow_state.set_step(state, "clean", "done", 0)

    pending_task_ids = tidy_flow_stages.list_task_ids(tasks_dir)
    blocked_task_id = pending_task_ids[0] if pending_task_ids else None
    if loop_ret == 2:
        final_exit_code = 2
    elif options.process_all and pending_task_ids:
        final_exit_code = 2
    else:
        final_exit_code = 0

    return {
        "phase_error": 0,
        "final_exit_code": final_exit_code,
        "pending_task_ids": pending_task_ids,
        "blocked_task_id": blocked_task_id,
    }
