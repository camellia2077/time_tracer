from collections.abc import Callable
import shutil
import subprocess
from pathlib import Path

from ...core.context import Context
from ...core.executor import run_command
from ...core.process_lock import ProcessLockBusyError, hold_process_lock
from . import common as build_common

_ANDROID_STABLE_GRADLE_ARGS = (
    "--no-configuration-cache",
    "--no-parallel",
    "--console=plain",
)


def _apply_android_gradle_guardrails(gradle_args: list[str]) -> list[str]:
    guarded = list(gradle_args)
    for arg in _ANDROID_STABLE_GRADLE_ARGS:
        if arg not in guarded:
            guarded.append(arg)
    return guarded


def _android_built_in_kotlinc_dirs(app_dir: Path) -> list[Path]:
    return sorted(
        path
        for path in app_dir.glob("*/build/intermediates/built_in_kotlinc")
        if path.is_dir()
    )


def _clear_android_built_in_kotlinc_dirs(app_dir: Path) -> int:
    cleared = 0
    for directory in _android_built_in_kotlinc_dirs(app_dir):
        try:
            shutil.rmtree(directory)
        except FileNotFoundError:
            continue
        except OSError as error:
            print(
                "--- build: failed to clear Android Kotlin intermediate "
                f"`{directory}`: {error}"
            )
            continue
        cleared += 1
    return cleared


def _android_install_requires_device(gradle_tasks: list[str]) -> bool:
    return any("install" in str(task).lower() for task in gradle_tasks)


def _ensure_android_device_connected(env: dict[str, str]) -> bool:
    adb_path = shutil.which("adb", path=env.get("PATH"))
    if adb_path is None:
        print(
            "--- build: ADB command not found; cannot install the Android APK. "
            "Please configure Android SDK platform-tools."
        )
        return False

    try:
        result = subprocess.run(
            [adb_path, "devices"],
            check=False,
            capture_output=True,
            text=True,
            env=env,
        )
    except OSError as error:
        print(f"--- build: failed to query ADB devices: {error}")
        return False

    connected_devices = [
        line
        for line in result.stdout.splitlines()
        if line.strip() and not line.startswith("List of devices")
        and len(line.split()) >= 2
        and line.split()[1] == "device"
    ]
    if result.returncode != 0 or not connected_devices:
        print(
            "--- build: ADB 没有任何连接设备（no connected devices）。"
            "请连接 Android 真机或启动模拟器后重试。"
        )
        return False
    return True


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
    if app_name == "tracer_android":
        # Android Gradle on Windows has repeatedly left `built_in_kotlinc` intermediates locked
        # across back-to-back verify/test runs. Prefer stable defaults here so verify can avoid
        # configuration-cache reuse races before falling back to a targeted intermediate cleanup.
        gradle_extra_args = _apply_android_gradle_guardrails(gradle_extra_args)
    if not build_common.resolve_android_native_optimization_gradle_args(gradle_extra_args):
        gradle_extra_args += build_common.resolve_android_native_optimization_gradle_property(
            ctx=ctx,
            app_name=app_name,
        )
    if app_name == "tracer_android":
        profile_label = profile_name or "default"
        print(
            "--- build: Android Gradle profile "
            f"`{profile_label}` tasks: {', '.join(gradle_tasks)}"
        )
    gradle_cmd = [
        build_common.resolve_gradle_wrapper(ctx, app_name),
        *gradle_tasks,
        *gradle_extra_args,
    ]
    app_dir = ctx.get_app_dir(app_name)
    command_env = ctx.setup_env()
    if app_name == "tracer_android" and _android_install_requires_device(gradle_tasks):
        if not _ensure_android_device_connected(command_env):
            return 1
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
            build_ret = effective_run_command(
                gradle_cmd,
                cwd=app_dir,
                env=command_env,
                log_file=log_file,
                output_mode=output_mode,
            )
            if build_ret == 0 or app_name != "tracer_android":
                return build_ret

            cleared_dirs = _clear_android_built_in_kotlinc_dirs(app_dir)
            if cleared_dirs <= 0:
                return build_ret

            print(
                "--- build: Android Gradle failed after Kotlin intermediate reuse; "
                f"cleared {cleared_dirs} `built_in_kotlinc` director"
                f"{'y' if cleared_dirs == 1 else 'ies'} and retrying once."
            )
            return effective_run_command(
                gradle_cmd,
                cwd=app_dir,
                env=command_env,
                log_file=log_file,
                output_mode=output_mode,
            )
    except ProcessLockBusyError as error:
        print(error.render_user_message())
        return 1
