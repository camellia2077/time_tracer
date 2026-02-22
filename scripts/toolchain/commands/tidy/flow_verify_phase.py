from ...core.context import Context
from . import flow_stages as tidy_flow_stages, flow_state as tidy_flow_state


def run_verify_phase(
    ctx: Context,
    options,
    state: dict,
    build_cmd,
    verify_build_dir_name: str,
) -> int:
    tidy_flow_state.set_phase(state, "verify")

    configure_ret = build_cmd.configure(
        app_name=options.app_name,
        tidy=False,
        extra_args=None,
        build_dir_name=verify_build_dir_name,
        profile_name=options.profile_name,
        kill_build_procs=options.kill_build_procs,
    )
    if configure_ret != 0:
        tidy_flow_state.set_step(state, "verify", "failed", configure_ret)
        return configure_ret

    build_ret = build_cmd.build(
        app_name=options.app_name,
        tidy=False,
        extra_args=None,
        build_dir_name=verify_build_dir_name,
        profile_name=options.profile_name,
        kill_build_procs=options.kill_build_procs,
    )
    if build_ret != 0:
        tidy_flow_state.set_step(state, "verify", "failed", build_ret)
        return build_ret

    suite_ret = tidy_flow_stages.run_suite_verify(
        ctx=ctx,
        app_name=options.app_name,
        build_dir_name=verify_build_dir_name,
        concise=options.concise,
    )
    if suite_ret != 0:
        tidy_flow_state.set_step(state, "verify", "failed", suite_ret)
        return suite_ret

    tidy_flow_state.set_step(state, "verify", "done", 0)
    return 0
