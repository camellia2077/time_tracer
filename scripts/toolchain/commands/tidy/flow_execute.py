from ...core.context import Context
from ..cmd_build import BuildCommand
from . import (
    flow_loop_clean_phase as tidy_flow_loop_clean_phase,
    flow_prepare_phase as tidy_flow_prepare_phase,
    flow_rename_phase as tidy_flow_rename_phase,
    flow_stages as tidy_flow_stages,
    flow_state as tidy_flow_state,
    flow_verify_phase as tidy_flow_verify_phase,
)


def _finish_with_pending_tasks(state: dict, state_path, exit_code: int, tasks_dir) -> int:
    return tidy_flow_state.finish(
        state=state,
        state_path=state_path,
        exit_code=exit_code,
        pending_task_ids=tidy_flow_stages.list_task_ids(tasks_dir),
    )


def execute_flow_impl(ctx: Context, options) -> int:
    if not options.process_all and options.n <= 0:
        print("--- tidy-flow: n <= 0, nothing to do.")
        return 0

    app_dir = ctx.get_app_dir(options.app_name)
    build_tidy_dir = app_dir / "build_tidy"
    tasks_dir = build_tidy_dir / "tasks"
    state_path = build_tidy_dir / "flow_state.json"
    effective_test_every = max(1, options.test_every)
    effective_n = options.n if options.n is not None else 1
    effective_keep_going = (
        ctx.config.tidy.keep_going if options.keep_going is None else options.keep_going
    )
    effective_run_tidy_fix = (
        ctx.config.tidy.run_fix_before_tidy
        if options.run_tidy_fix is None
        else options.run_tidy_fix
    )
    effective_tidy_fix_limit = (
        ctx.config.tidy.fix_limit if options.tidy_fix_limit is None else options.tidy_fix_limit
    )

    build_cmd = BuildCommand(ctx)
    verify_build_dir_name = build_cmd.resolve_build_dir_name(
        tidy=False,
        build_dir_name=options.build_dir_name,
        profile_name=options.profile_name,
        app_name=options.app_name,
    )

    state = tidy_flow_state.new_state(
        app_name=options.app_name,
        process_all=options.process_all,
        n=effective_n,
        resume=options.resume,
        test_every=effective_test_every,
        concise=options.concise,
        jobs=options.jobs,
        parse_workers=options.parse_workers,
        keep_going=effective_keep_going,
        run_tidy_fix=effective_run_tidy_fix,
        tidy_fix_limit=effective_tidy_fix_limit,
        verify_build_dir=verify_build_dir_name,
        profile_name=options.profile_name,
        kill_build_procs=options.kill_build_procs,
        state_path=state_path,
    )

    prepare_ret = tidy_flow_prepare_phase.run_prepare_phase(
        ctx=ctx,
        options=options,
        state=state,
        tasks_dir=tasks_dir,
        effective_run_tidy_fix=effective_run_tidy_fix,
        effective_tidy_fix_limit=effective_tidy_fix_limit,
        effective_keep_going=effective_keep_going,
    )
    if prepare_ret != 0:
        return _finish_with_pending_tasks(state, state_path, prepare_ret, tasks_dir)

    rename_ret = tidy_flow_rename_phase.run_rename_phase(
        ctx=ctx,
        options=options,
        state=state,
        build_tidy_dir=build_tidy_dir,
    )
    if rename_ret != 0:
        return _finish_with_pending_tasks(state, state_path, rename_ret, tasks_dir)

    verify_ret = tidy_flow_verify_phase.run_verify_phase(
        ctx=ctx,
        options=options,
        state=state,
        build_cmd=build_cmd,
        verify_build_dir_name=verify_build_dir_name,
    )
    if verify_ret != 0:
        return _finish_with_pending_tasks(state, state_path, verify_ret, tasks_dir)

    loop_result = tidy_flow_loop_clean_phase.run_loop_clean_phase(
        ctx=ctx,
        options=options,
        state=state,
        tasks_dir=tasks_dir,
        effective_n=effective_n,
        effective_test_every=effective_test_every,
    )
    if loop_result["phase_error"] != 0:
        return _finish_with_pending_tasks(
            state,
            state_path,
            loop_result["phase_error"],
            tasks_dir,
        )

    return tidy_flow_state.finish(
        state=state,
        state_path=state_path,
        exit_code=loop_result["final_exit_code"],
        pending_task_ids=loop_result["pending_task_ids"],
        blocked_task_id=loop_result["blocked_task_id"],
    )
