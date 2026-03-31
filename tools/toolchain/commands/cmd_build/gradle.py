from collections.abc import Callable

from ...core.context import Context
from ...core.executor import run_command
from ...core.process_lock import ProcessLockBusyError, hold_process_lock
from . import common as build_common


def configure_gradle(
    app_name: str,
    tidy: bool,
    extra_args: list[str] | None,
    cmake_args: list[str] | None,
    build_dir_name: str | None,
    log_file=None,
    output_mode: str = "live",
) -> int:
    _ = log_file, output_mode
    if tidy:
        print("--- configure: gradle backend does not use `--tidy`; flag ignored.")
    if build_dir_name and build_dir_name != "build":
        print(f"--- configure: gradle backend ignores --build-dir ({build_dir_name}).")
    filtered_extra_args = [a for a in (extra_args or []) if a != "--"]
    filtered_cmake_args = [a for a in (cmake_args or []) if a != "--"]
    if filtered_extra_args or filtered_cmake_args:
        print("--- configure: gradle backend does not accept CMake args; extra args ignored.")
    print("--- configure: app uses gradle backend; configure stage skipped.")
    return 0


def build_gradle(
    ctx: Context,
    app_name: str,
    tidy: bool,
    extra_args: list[str] | None,
    cmake_args: list[str] | None,
    build_dir_name: str | None,
    profile_name: str | None,
    run_command_fn: Callable[..., int] | None = None,
    log_file=None,
    output_mode: str = "live",
) -> int:
    effective_run_command = run_command if run_command_fn is None else run_command_fn
    if tidy:
        print("--- build: gradle backend does not use `--tidy`; flag ignored.")
    if build_dir_name and build_dir_name != "build":
        print(f"--- build: gradle backend ignores --build-dir ({build_dir_name}).")
    filtered_cmake_args = [a for a in (cmake_args or []) if a != "--"]
    if filtered_cmake_args:
        print("--- build: gradle backend ignores --cmake-args.")

    gradle_tasks = build_common.resolve_gradle_tasks(
        ctx=ctx,
        app_name=app_name,
        profile_name=profile_name,
    )
    gradle_extra_args = [a for a in (extra_args or []) if a != "--"]
    for profile_gradle_arg in build_common.profile_gradle_args(ctx, profile_name):
        if profile_gradle_arg not in gradle_extra_args:
            gradle_extra_args.append(profile_gradle_arg)
    if not build_common.resolve_android_native_optimization_gradle_args(gradle_extra_args):
        gradle_extra_args += build_common.resolve_android_native_optimization_gradle_property(
            ctx=ctx,
            app_name=app_name,
        )
    gradle_cmd = [
        build_common.resolve_gradle_wrapper(ctx, app_name),
        *gradle_tasks,
        *gradle_extra_args,
    ]
    app_dir = ctx.get_app_dir(app_name)
    lock_path = ctx.get_out_root() / "locks" / app_name / "android_gradle.lock"
    lock_metadata = {
        "command": " ".join(str(token) for token in gradle_cmd),
        "cwd": app_dir.as_posix(),
    }
    try:
        with hold_process_lock(
            lock_path=lock_path,
            label=f"Android Gradle command for `{app_name}`",
            metadata=lock_metadata,
        ):
            return effective_run_command(
                gradle_cmd,
                cwd=app_dir,
                env=ctx.setup_env(),
                log_file=log_file,
                output_mode=output_mode,
            )
    except ProcessLockBusyError as error:
        print(error.render_user_message())
        return 1
