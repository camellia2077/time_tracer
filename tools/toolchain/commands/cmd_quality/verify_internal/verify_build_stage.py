from __future__ import annotations

import time

from ....services.suite_registry import resolve_suite_build_app
from ...cmd_build import BuildCommand


def _resolve_runtime_platform(app_name: str) -> str | None:
    if app_name == "tracer_windows_rust_cli":
        return "windows"
    return None


def execute_build_stage(
    *,
    ctx,
    build_command_cls=BuildCommand,
    app_name: str,
    tidy: bool,
    extra_args: list[str] | None,
    cmake_args: list[str] | None,
    build_dir_name: str | None,
    profile_name: str | None,
    concise: bool,
    kill_build_procs: bool,
    run_command_fn=None,
) -> tuple[int, str, str, object]:
    build_app_name = resolve_suite_build_app(app_name) or app_name
    build_cmd = build_command_cls(ctx)

    # tracer_core/tracer_core_shell verify flow needs fresh core runtime
    # artifacts before building the Rust CLI shell.
    if app_name in {"tracer_core", "tracer_core_shell"} and build_app_name == "tracer_windows_rust_cli":
        core_build_ret = build_cmd.build(
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            concise=concise,
            kill_build_procs=kill_build_procs,
            run_command_fn=run_command_fn,
        )
        if core_build_ret != 0:
            resolved_core_build_dir_name = build_cmd.resolve_build_dir_name(
                tidy=tidy,
                build_dir_name=build_dir_name,
                profile_name=profile_name,
                app_name=app_name,
            )
            core_log_path = build_cmd.resolve_output_log_path(
                app_name=app_name,
                tidy=tidy,
                build_dir_name=build_dir_name,
                profile_name=profile_name,
            )
            return int(core_build_ret), resolved_core_build_dir_name, app_name, core_log_path

    build_ret = build_cmd.build(
        app_name=build_app_name,
        tidy=tidy,
        extra_args=extra_args,
        cmake_args=cmake_args,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        concise=concise,
        runtime_platform=_resolve_runtime_platform(build_app_name),
        kill_build_procs=kill_build_procs,
        run_command_fn=run_command_fn,
    )
    resolved_build_dir_name = build_cmd.resolve_build_dir_name(
        tidy=tidy,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        app_name=build_app_name,
    )
    build_log_path = build_cmd.resolve_output_log_path(
        app_name=build_app_name,
        tidy=tidy,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
    )
    return int(build_ret), resolved_build_dir_name, build_app_name, build_log_path


def handle_post_build_state(
    *,
    suite_name: str | None,
    verify_scope: str,
    build_ret: int,
    app_name: str,
    resolved_build_dir_name: str,
    started_at: float,
    verify_command_text: str,
    repo_root,
    write_build_only_result_json_fn,
    print_failure_report_fn,
    print_result_paths_fn,
    build_log_path,
) -> int | None:
    if build_ret != 0:
        write_build_only_result_json_fn(
            app_name=app_name,
            build_dir_name=resolved_build_dir_name,
            success=False,
            exit_code=build_ret,
            duration_seconds=time.monotonic() - started_at,
            error_message="Build failed during verify.",
            build_only=(suite_name is None),
        )
        print_failure_report_fn(
            command=verify_command_text,
            exit_code=build_ret,
            next_action=f"Fix errors and rerun: {verify_command_text}",
            app_name=app_name,
            repo_root=repo_root,
            log_path=build_log_path,
            fallback_key_error_hint="Build failed. See command output above.",
        )
        print_result_paths_fn(app_name=app_name, repo_root=repo_root)
        return build_ret

    if suite_name is None and verify_scope != "unit":
        write_build_only_result_json_fn(
            app_name=app_name,
            build_dir_name=resolved_build_dir_name,
            success=True,
            exit_code=0,
            duration_seconds=time.monotonic() - started_at,
        )
        print_result_paths_fn(app_name=app_name, repo_root=repo_root)
        return 0

    return None
