"""Inject repository test data into a debuggable Android installation."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path, PurePosixPath


DEFAULT_PACKAGE = "com.example.tracer"
REMOTE_STAGE = "/data/local/tmp/time_tracer_test_data"


def adb_command(serial: str | None, *args: str) -> list[str]:
    command = ["adb"]
    if serial:
        command.extend(["-s", serial])
    command.extend(args)
    return command


def run_adb(serial: str | None, *args: str, capture: bool = False) -> str:
    stdout = subprocess.PIPE if capture else None
    stderr = subprocess.STDOUT if capture else None
    completed = subprocess.run(
        adb_command(serial, *args),
        check=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        stdout=stdout,
        stderr=stderr,
    )
    return completed.stdout.strip() if capture else ""


def private_shell(serial: str | None, package: str, command: str) -> str:
    # `adb shell` performs its own argument splitting. Keep the command as one
    # quoted shell argument so that `sh -c` receives the complete expression.
    quoted_command = command.replace('"', '\\"')
    return run_adb(
        serial,
        "shell",
        f'run-as {package} sh -c "{quoted_command}"',
        capture=True,
    )


def files_under(root: Path, suffix: str) -> list[tuple[Path, PurePosixPath]]:
    if not root.is_dir():
        raise ValueError(f"Missing data directory: {root}")
    files: list[tuple[Path, PurePosixPath]] = []
    for path in sorted(root.rglob(f"*{suffix}")):
        if path.is_file():
            relative = PurePosixPath(path.relative_to(root).as_posix())
            files.append((path, relative))
    if not files:
        raise ValueError(f"No {suffix} files found under {root}")
    return files


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Push test TXT and activity_hierarchy TOML data into the app's "
            "private runtime directory. The APK must already be installed."
        )
    )
    parser.add_argument("--serial", help="ADB device serial; required when multiple devices are connected")
    parser.add_argument("--package", default=DEFAULT_PACKAGE, help=f"Application id (default: {DEFAULT_PACKAGE})")
    parser.add_argument("--txt-root", type=Path, default=Path("test/data"))
    parser.add_argument(
        "--activity-hierarchy-root",
        type=Path,
        default=Path("test/data/activity_hierarchy"),
    )
    parser.add_argument(
        "--keep-database",
        action="store_true",
        help="Keep the existing private database instead of starting from a clean test database",
    )
    parser.add_argument("--launch", action="store_true", help="Launch the app after injection")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    adb = shutil.which("adb")
    if not adb:
        raise RuntimeError("adb was not found on PATH")

    txt_files = files_under(args.txt_root, ".txt")
    hierarchy_files = [
        (local, relative)
        for local, relative in files_under(args.activity_hierarchy_root, ".toml")
    ]
    if not hierarchy_files:
        raise ValueError(
            f"No activity hierarchy TOML files found under {args.activity_hierarchy_root}"
        )

    run_adb(args.serial, "wait-for-device")
    installed = run_adb(args.serial, "shell", "pm", "path", args.package, capture=True)
    if not installed:
        raise RuntimeError(f"Package {args.package} is not installed; build/install the debug APK first")

    run_adb(args.serial, "shell", "am", "force-stop", args.package)
    run_adb(args.serial, "shell", "rm", "-rf", REMOTE_STAGE)
    try:
        for local, relative in txt_files:
            remote = f"{REMOTE_STAGE}/input/{relative.as_posix()}"
            run_adb(args.serial, "shell", "mkdir", "-p", str(PurePosixPath(remote).parent))
            run_adb(args.serial, "push", str(local), remote)
        for local, relative in hierarchy_files:
            remote = f"{REMOTE_STAGE}/activity_hierarchy/{relative.as_posix()}"
            run_adb(args.serial, "shell", "mkdir", "-p", str(PurePosixPath(remote).parent))
            run_adb(args.serial, "push", str(local), remote)

        clean_paths = [
            "files/input",
            "files/config/user/activity_hierarchy",
            # Remove the pre-layout runtime tree so stale data cannot make the
            # app appear populated while the new filesDir layout is empty.
            "files/tracer_core",
        ]
        if not args.keep_database:
            clean_paths.append("files/db")
        clean_paths.append("files/.data_folder_snapshot")
        private_shell(
            args.serial,
            args.package,
            "rm -rf " + " ".join(clean_paths) +
            " && mkdir -p files/input files/config/user/activity_hierarchy",
        )

        for _, relative in txt_files:
            private_shell(
                args.serial,
                args.package,
                f"mkdir -p files/input/{relative.parent.as_posix()} && "
                f"cp {REMOTE_STAGE}/input/{relative.as_posix()} files/input/{relative.as_posix()}",
            )
        for _, relative in hierarchy_files:
            private_shell(
                args.serial,
                args.package,
                f"mkdir -p files/config/user/activity_hierarchy/{relative.parent.as_posix()} && "
                f"cp {REMOTE_STAGE}/activity_hierarchy/{relative.as_posix()} "
                f"files/config/user/activity_hierarchy/{relative.as_posix()}",
            )

        txt_count = private_shell(
            args.serial,
            args.package,
            "find files/input -type f -name '*.txt' | wc -l",
        )
        hierarchy_count = private_shell(
            args.serial,
            args.package,
            "find files/config/user/activity_hierarchy -type f -name '*.toml' | wc -l",
        )
        expected_txt = len(txt_files)
        expected_hierarchy = len(hierarchy_files)
        if int(txt_count) != expected_txt or int(hierarchy_count) != expected_hierarchy:
            raise RuntimeError(
                f"Injected file count mismatch: TXT {txt_count}/{expected_txt}, "
                f"activity_hierarchy {hierarchy_count}/{expected_hierarchy}"
            )
    finally:
        run_adb(args.serial, "shell", "rm", "-rf", REMOTE_STAGE)

    if args.launch:
        run_adb(args.serial, "shell", "monkey", "-p", args.package, "1")
    print(
        f"Injected {expected_txt} TXT files and {expected_hierarchy} activity_hierarchy TOML files "
        f"into {args.package}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        output = (error.stdout or "").strip()
        suffix = f"\nadb output:\n{output}" if output else ""
        print(f"error: {error}{suffix}", file=sys.stderr)
        raise SystemExit(1)
    except (RuntimeError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
