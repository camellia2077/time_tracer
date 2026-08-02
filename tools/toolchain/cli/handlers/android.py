import argparse
import shutil
import subprocess
import sys
from pathlib import Path

from ...commands.cmd_build import BuildCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


ANDROID_PACKAGE = "com.example.tracer"
ANDROID_TEST_DATA_SCRIPT = Path("tools/scripts/devtools/android/push_test_data.py")
ANDROID_REBUILD_DATABASE_ACTION = "com.example.tracer.action.REBUILD_DATABASE"


def _adb_command(serial: str | None, *args: str) -> list[str]:
    command = ["adb"]
    if serial:
        command.extend(["-s", serial])
    command.extend(args)
    return command


def _apk_path(ctx: Context, variant: str) -> Path:
    if variant == "debug":
        return ctx.get_app_dir("tracer_android") / "app/build/outputs/apk/debug/app-debug.apk"
    return ctx.get_app_dir("tracer_android") / "app/build/outputs/final-apk/release/Tracer-release.apk"


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument(
        "--variant",
        choices=("debug", "release"),
        default="debug",
        help="Android variant to build or install (default: debug).",
    )
    parser.add_argument(
        "--install",
        action="store_true",
        help="Install the APK after building it.",
    )
    parser.add_argument(
        "--install-only",
        action="store_true",
        help="Install an already-built APK without building it.",
    )
    parser.add_argument(
        "--serial",
        help="ADB device serial; required when multiple devices are connected.",
    )
    parser.add_argument(
        "--with-test-data",
        action="store_true",
        help="Inject test/data and test/data/activity_hierarchy after a debug install.",
    )
    parser.add_argument(
        "--keep-database",
        action="store_true",
        help="Keep the existing database when --with-test-data is used.",
    )
    parser.add_argument(
        "--rebuild-database",
        action="store_true",
        help="Rebuild the debug database from all TXT files through the Android runtime.",
    )


def _build(args: argparse.Namespace, ctx: Context) -> int:
    profile_name = "android_edit" if args.variant == "debug" else "android_release"
    return BuildCommand(ctx).build(
        app_name="tracer_android",
        tidy=False,
        profile_name=profile_name,
    )


def _install(args: argparse.Namespace, ctx: Context) -> int:
    if (args.with_test_data or args.rebuild_database) and args.variant != "debug":
        print(
            "Error: --with-test-data and --rebuild-database require --variant debug.",
            file=sys.stderr,
        )
        return 2

    adb_path = shutil.which("adb", path=ctx.setup_env().get("PATH"))
    if adb_path is None:
        print("Error: ADB was not found on PATH.", file=sys.stderr)
        return 1

    apk_path = _apk_path(ctx, args.variant)
    if not apk_path.is_file():
        print(f"Error: Android APK was not produced: {apk_path}", file=sys.stderr)
        return 1

    install_command = _adb_command(args.serial, "install", "-r", str(apk_path))
    install_command[0] = adb_path
    installed = subprocess.run(install_command, cwd=ctx.repo_root, check=False)
    if installed.returncode != 0:
        return installed.returncode

    if args.with_test_data:
        test_data_command = [sys.executable, str(ctx.repo_root / ANDROID_TEST_DATA_SCRIPT)]
        if args.serial:
            test_data_command.extend(["--serial", args.serial])
        if args.keep_database:
            test_data_command.append("--keep-database")
        if not (args.rebuild_database or args.with_test_data):
            test_data_command.append("--launch")
        injected = subprocess.run(test_data_command, cwd=ctx.repo_root, check=False)
        if injected.returncode != 0:
            return injected.returncode

    if args.rebuild_database or args.with_test_data:
        rebuilt = _rebuild_database(args, ctx, adb_path)
        if rebuilt != 0:
            return rebuilt
        _launch_app(args, ctx, adb_path)

    print(f"Android {args.variant} APK installed successfully.")
    return 0


def _launch_app(args: argparse.Namespace, ctx: Context, adb_path: str) -> None:
    command = _adb_command(args.serial, "shell", "monkey", "-p", ANDROID_PACKAGE, "1")
    command[0] = adb_path
    subprocess.run(command, cwd=ctx.repo_root, check=False)


def _rebuild_database(args: argparse.Namespace, ctx: Context, adb_path: str) -> int:
    command = _adb_command(
        args.serial,
        "shell",
        "am",
        "broadcast",
        "--receiver-foreground",
        "-n",
        f"{ANDROID_PACKAGE}/.DebugCommandReceiver",
        "-a",
        ANDROID_REBUILD_DATABASE_ACTION,
    )
    command[0] = adb_path
    completed = subprocess.run(
        command,
        cwd=ctx.repo_root,
        check=False,
        text=True,
        capture_output=True,
    )
    output = (completed.stdout or "").strip()
    if output:
        print(output)
    if completed.returncode != 0:
        return completed.returncode
    if "result=0" not in output:
        print("Error: Android database rebuild failed.", file=sys.stderr)
        return 1
    print("Android database rebuilt successfully.")
    return 0


def run(args: argparse.Namespace, ctx: Context) -> int:
    if (args.with_test_data or args.rebuild_database) and args.variant != "debug":
        print(
            "Error: --with-test-data and --rebuild-database require --variant debug.",
            file=sys.stderr,
        )
        return 2
    if args.install_only and not args.install:
        args.install = True
    if args.with_test_data and not args.install:
        print("Error: --with-test-data requires --install.", file=sys.stderr)
        return 2
    if args.keep_database and not args.with_test_data:
        print("Error: --keep-database requires --with-test-data.", file=sys.stderr)
        return 2
    if args.rebuild_database and not (args.install or args.install_only):
        print(
            "Error: --rebuild-database requires --install or --install-only.",
            file=sys.stderr,
        )
        return 2
    if args.install_only:
        return _install(args, ctx)
    build_ret = _build(args, ctx)
    if build_ret != 0:
        return build_ret
    if args.install:
        return _install(args, ctx)
    return 0


COMMAND = CommandSpec(
    name="android",
    register=register,
    run=run,
    app_mode="none",
    add_app_path=False,
    help="Build Android APK, optionally install it and inject test data.",
)
