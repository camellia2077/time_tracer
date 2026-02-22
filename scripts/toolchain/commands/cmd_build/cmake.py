from collections.abc import Callable
from pathlib import Path

from ...core.context import Context
from . import common as build_common


def is_configured(
    ctx: Context,
    app_name: str,
    tidy: bool,
    build_dir_name: str | None,
    profile_name: str | None,
    resolve_backend_fn: Callable[[str], str],
    resolve_build_dir_name_fn: Callable[[bool, str | None, str | None, str | None], str],
) -> bool:
    if resolve_backend_fn(app_name) == "gradle":
        return True
    resolved_build_dir_name = resolve_build_dir_name_fn(
        tidy,
        build_dir_name,
        profile_name,
        app_name,
    )
    build_dir = ctx.get_app_dir(app_name) / resolved_build_dir_name
    return (build_dir / "CMakeCache.txt").exists()


def needs_windows_config_reconfigure(ctx: Context, app_name: str, build_dir: Path) -> bool:
    if build_common.resolve_config_sync_target(ctx, app_name) != "windows":
        return False
    cache_path = build_dir / "CMakeCache.txt"
    if not cache_path.exists():
        return True

    expected = build_common.normalize_cache_path(
        str(build_common.resolve_platform_config_output_root(ctx, "windows").resolve())
    )
    try:
        for raw_line in cache_path.read_text(encoding="utf-8").splitlines():
            if not raw_line.startswith("TRACER_WINDOWS_CONFIG_SOURCE_DIR:"):
                continue
            _, _, value = raw_line.partition("=")
            if not value.strip():
                return True
            actual = build_common.normalize_cache_path(value)
            return actual != expected
    except OSError:
        return True
    return True


def configure_cmake(
    ctx: Context,
    app_name: str,
    tidy: bool,
    extra_args: list[str] | None,
    cmake_args: list[str] | None,
    build_dir_name: str | None,
    profile_name: str | None,
    resolve_build_dir_name_fn: Callable[[bool, str | None, str | None, str | None], str],
    run_command_fn: Callable[..., int],
) -> int:
    app = ctx.get_app_metadata(app_name)
    resolved_build_dir_name = resolve_build_dir_name_fn(
        tidy,
        build_dir_name,
        profile_name,
        app_name,
    )
    build_dir = ctx.get_app_dir(app_name) / resolved_build_dir_name
    source_dir = ctx.get_app_dir(app_name)

    build_dir.mkdir(parents=True, exist_ok=True)

    flags = app.cmake_flags[:]
    if tidy:
        flags.extend(["-D", "ENABLE_CLANG_TIDY=ON", "-D", "ENABLE_PCH=OFF"])
    else:
        flags.extend(["-D", "ENABLE_CLANG_TIDY=OFF", "-D", "ENABLE_PCH=ON"])

    profile_cmake_args = build_common.profile_cmake_args(ctx, profile_name)
    filtered_extra_args = [a for a in (extra_args or []) if a != "--"]
    filtered_cmake_args = [a for a in (cmake_args or []) if a != "--"]
    configure_args = profile_cmake_args + filtered_extra_args + filtered_cmake_args
    is_valid_override, error_message = build_common.validate_windows_config_source_override(
        ctx=ctx,
        app_name=app_name,
        configure_args=configure_args,
    )
    if not is_valid_override:
        print(error_message)
        return 2
    configure_args = build_common.strip_cmake_definition(
        configure_args, "TRACER_WINDOWS_CONFIG_SOURCE_DIR"
    )
    configure_args += build_common.resolve_windows_config_cmake_args(ctx, app_name)
    toolchain_flags = build_common.resolve_toolchain_flags(ctx, flags + configure_args)
    config_cmd = (
        ["cmake", "-S", str(source_dir), "-B", str(build_dir)]
        + flags
        + toolchain_flags
        + configure_args
    )
    return run_command_fn(config_cmd, env=ctx.setup_env())


def build_cmake(
    ctx: Context,
    app_name: str,
    tidy: bool,
    extra_args: list[str] | None,
    cmake_args: list[str] | None,
    build_dir_name: str | None,
    profile_name: str | None,
    resolve_build_dir_name_fn: Callable[[bool, str | None, str | None, str | None], str],
    is_configured_fn: Callable[[str, bool, str | None, str | None], bool],
    needs_windows_config_reconfigure_fn: Callable[[str, Path], bool],
    configure_fn: Callable[..., int],
    sync_windows_runtime_config_copy_if_needed_fn: Callable[[str, str], int],
    run_command_fn: Callable[..., int],
) -> int:
    resolved_build_dir_name = resolve_build_dir_name_fn(
        tidy,
        build_dir_name,
        profile_name,
        app_name,
    )
    build_dir = ctx.get_app_dir(app_name) / resolved_build_dir_name
    filtered_build_args = [a for a in (extra_args or []) if a != "--"]
    filtered_cmake_args = [a for a in (cmake_args or []) if a != "--"]
    explicit_profile_requested = bool((profile_name or "").strip())
    is_currently_configured = is_configured_fn(
        app_name,
        tidy,
        build_dir_name,
        profile_name,
    )
    should_configure = (
        filtered_cmake_args
        or explicit_profile_requested
        or not is_currently_configured
        or needs_windows_config_reconfigure_fn(app_name, build_dir)
    )
    if should_configure:
        if not is_currently_configured:
            print(f"--- build: {build_dir} is not configured. Running auto-configure...")
        configure_ret = configure_fn(
            app_name=app_name,
            tidy=tidy,
            extra_args=None,
            cmake_args=filtered_cmake_args,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            kill_build_procs=False,
            sync_platform_config=False,
        )
        if configure_ret != 0:
            return configure_ret
    build_cmd = ["cmake", "--build", str(build_dir), "-j"] + filtered_build_args
    build_ret = run_command_fn(build_cmd, env=ctx.setup_env())
    if build_ret != 0:
        return build_ret
    return sync_windows_runtime_config_copy_if_needed_fn(
        app_name=app_name,
        build_dir_name=resolved_build_dir_name,
    )
