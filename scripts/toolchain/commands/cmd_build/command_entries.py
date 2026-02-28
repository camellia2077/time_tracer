from . import cmake as build_cmake, gradle as build_gradle


def configure_entry(
    command,
    app_name: str,
    tidy: bool,
    extra_args: list[str] | None = None,
    cmake_args: list[str] | None = None,
    build_dir_name: str | None = None,
    profile_name: str | None = None,
    kill_build_procs: bool = False,
    sync_platform_config: bool = True,
    run_command_fn=None,
    kill_build_processes_fn=None,
) -> int:
    if kill_build_procs:
        kill_build_processes_fn()

    if sync_platform_config:
        sync_ret = command._sync_platform_config_if_needed(app_name)
        if sync_ret != 0:
            return sync_ret

    backend = command._resolve_backend(app_name)
    if backend == "gradle":
        return build_gradle.configure_gradle(
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
        )

    return build_cmake.configure_cmake(
        ctx=command.ctx,
        app_name=app_name,
        tidy=tidy,
        extra_args=extra_args,
        cmake_args=cmake_args,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        resolve_build_dir_name_fn=command.resolve_build_dir_name,
        run_command_fn=run_command_fn,
    )


def build_entry(
    command,
    app_name: str,
    tidy: bool,
    extra_args: list[str] | None = None,
    cmake_args: list[str] | None = None,
    build_dir_name: str | None = None,
    profile_name: str | None = None,
    kill_build_procs: bool = False,
    run_command_fn=None,
    kill_build_processes_fn=None,
    kill_runtime_lock_processes_fn=None,
) -> int:
    if app_name in {"tracer_core", "tracer_windows_cli"} and kill_runtime_lock_processes_fn:
        kill_runtime_lock_processes_fn()

    if kill_build_procs:
        kill_build_processes_fn()

    sync_ret = command._sync_platform_config_if_needed(app_name)
    if sync_ret != 0:
        return sync_ret

    backend = command._resolve_backend(app_name)
    if backend == "gradle":
        return build_gradle.build_gradle(
            ctx=command.ctx,
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            run_command_fn=run_command_fn,
        )

    return build_cmake.build_cmake(
        ctx=command.ctx,
        app_name=app_name,
        tidy=tidy,
        extra_args=extra_args,
        cmake_args=cmake_args,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        resolve_build_dir_name_fn=command.resolve_build_dir_name,
        is_configured_fn=command._is_configured,
        needs_windows_config_reconfigure_fn=command._needs_windows_config_reconfigure,
        configure_fn=command.configure,
        sync_windows_runtime_config_copy_if_needed_fn=command._sync_windows_runtime_config_copy_if_needed,
        run_command_fn=run_command_fn,
    )


def execute_entry(
    command,
    app_name: str,
    tidy: bool,
    extra_args: list[str] | None = None,
    cmake_args: list[str] | None = None,
    build_dir_name: str | None = None,
    profile_name: str | None = None,
    kill_build_procs: bool = False,
    kill_build_processes_fn=None,
) -> int:
    if kill_build_procs:
        kill_build_processes_fn()

    ret = command.configure(
        app_name,
        tidy,
        extra_args,
        cmake_args=cmake_args,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        kill_build_procs=False,
        sync_platform_config=True,
    )
    if ret != 0:
        return ret
    return command.build(
        app_name,
        tidy,
        extra_args,
        cmake_args=None,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        kill_build_procs=False,
    )
