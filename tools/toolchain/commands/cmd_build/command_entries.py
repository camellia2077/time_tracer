from . import cargo as build_cargo, cmake as build_cmake, gradle as build_gradle


def _print_build_output_dir(
    command,
    app_name: str,
    backend: str,
    build_dir_name: str,
    tidy: bool,
) -> None:
    if backend == "gradle":
        build_root = (command.ctx.get_app_dir(app_name) / "app" / "build" / "outputs" / "apk" / "release").resolve()
    elif tidy:
        build_root = command.ctx.get_tidy_dir(app_name, build_dir_name)
    else:
        build_root = command.ctx.get_build_dir(app_name, build_dir_name)
    print(f"Build files have been written to: {build_root.as_posix()}")


def configure_entry(
    command,
    app_name: str,
    tidy: bool,
    source_scope: str | None = None,
    config_file: str | None = None,
    strict_config: bool = False,
    extra_args: list[str] | None = None,
    cmake_args: list[str] | None = None,
    build_dir_name: str | None = None,
    profile_name: str | None = None,
    concise: bool = False,
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
    output_log_path = command.resolve_output_log_path(
        app_name=app_name,
        tidy=tidy,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
    )
    output_mode = "quiet" if concise else "live"
    if backend == "gradle":
        return build_gradle.configure_gradle(
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
            log_file=output_log_path,
            output_mode=output_mode,
        )
    if backend == "cargo":
        return build_cargo.configure_cargo(
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
            log_file=output_log_path,
            output_mode=output_mode,
        )

    return build_cmake.configure_cmake(
        ctx=command.ctx,
        app_name=app_name,
        tidy=tidy,
        source_scope=source_scope,
        config_file=config_file,
        strict_config=strict_config,
        extra_args=extra_args,
        cmake_args=cmake_args,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        resolve_build_dir_name_fn=command.resolve_build_dir_name,
        run_command_fn=run_command_fn,
        log_file=output_log_path,
        output_mode=output_mode,
    )


def build_entry(
    command,
    app_name: str,
    tidy: bool,
    source_scope: str | None = None,
    config_file: str | None = None,
    strict_config: bool = False,
    extra_args: list[str] | None = None,
    cmake_args: list[str] | None = None,
    build_dir_name: str | None = None,
    profile_name: str | None = None,
    concise: bool = False,
    windows_icon_svg: str | None = None,
    rust_runtime_sync: str | None = None,
    runtime_platform: str | None = None,
    kill_build_procs: bool = False,
    run_command_fn=None,
    kill_build_processes_fn=None,
    kill_runtime_lock_processes_fn=None,
) -> int:
    if app_name in {"tracer_core", "tracer_core_shell", "tracer_windows_rust_cli"} and kill_runtime_lock_processes_fn:
        kill_runtime_lock_processes_fn()

    if kill_build_procs:
        kill_build_processes_fn()

    sync_ret = command._sync_platform_config_if_needed(app_name)
    if sync_ret != 0:
        return sync_ret

    backend = command._resolve_backend(app_name)
    resolved_build_dir_name = command.resolve_build_dir_name(
        tidy=tidy,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        app_name=app_name,
    )
    output_log_path = command.resolve_output_log_path(
        app_name=app_name,
        tidy=tidy,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
    )
    output_mode = "quiet" if concise else "live"
    if backend == "gradle":
        ret = build_gradle.build_gradle(
            ctx=command.ctx,
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            run_command_fn=run_command_fn,
            log_file=output_log_path,
            output_mode=output_mode,
        )
        if ret == 0:
            _print_build_output_dir(
                command=command,
                app_name=app_name,
                backend=backend,
                build_dir_name=resolved_build_dir_name,
                tidy=tidy,
            )
        return ret
    if backend == "cargo":
        ret = build_cargo.build_cargo(
            ctx=command.ctx,
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=resolved_build_dir_name,
            profile_name=profile_name,
            windows_icon_svg=windows_icon_svg,
            rust_runtime_sync=rust_runtime_sync,
            runtime_platform=runtime_platform,
            run_command_fn=run_command_fn,
            log_file=output_log_path,
            output_mode=output_mode,
        )
        if ret == 0:
            _print_build_output_dir(
                command=command,
                app_name=app_name,
                backend=backend,
                build_dir_name=resolved_build_dir_name,
                tidy=tidy,
            )
        return ret

    ret = build_cmake.build_cmake(
        ctx=command.ctx,
        app_name=app_name,
        tidy=tidy,
        source_scope=source_scope,
        config_file=config_file,
        strict_config=strict_config,
        extra_args=extra_args,
        cmake_args=cmake_args,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        runtime_platform=runtime_platform,
        resolve_build_dir_name_fn=command.resolve_build_dir_name,
        is_configured_fn=command._is_configured,
        needs_windows_config_reconfigure_fn=command._needs_windows_config_reconfigure,
        configure_fn=command.configure,
        sync_windows_runtime_config_copy_if_needed_fn=command._sync_windows_runtime_config_copy_if_needed,
        run_command_fn=run_command_fn,
        log_file=output_log_path,
        output_mode=output_mode,
        concise=concise,
    )
    if ret == 0:
        _print_build_output_dir(
            command=command,
            app_name=app_name,
            backend=backend,
            build_dir_name=resolved_build_dir_name,
            tidy=tidy,
        )
    return ret


def execute_entry(
    command,
    app_name: str,
    tidy: bool,
    source_scope: str | None = None,
    config_file: str | None = None,
    strict_config: bool = False,
    extra_args: list[str] | None = None,
    cmake_args: list[str] | None = None,
    build_dir_name: str | None = None,
    profile_name: str | None = None,
    concise: bool = False,
    kill_build_procs: bool = False,
    kill_build_processes_fn=None,
    run_command_fn=None,
) -> int:
    if kill_build_procs:
        kill_build_processes_fn()

    ret = command.configure(
        app_name,
        tidy,
        source_scope=source_scope,
        config_file=config_file,
        strict_config=strict_config,
        extra_args=extra_args,
        cmake_args=cmake_args,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        concise=concise,
        kill_build_procs=False,
        sync_platform_config=True,
        run_command_fn=run_command_fn,
    )
    if ret != 0:
        return ret
    return command.build(
        app_name,
        tidy,
        source_scope=source_scope,
        config_file=config_file,
        strict_config=strict_config,
        extra_args=extra_args,
        cmake_args=None,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
        concise=concise,
        kill_build_procs=False,
        run_command_fn=run_command_fn,
    )
