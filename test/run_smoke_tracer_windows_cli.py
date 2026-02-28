#!/usr/bin/env python3
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def _run(command: list[str], repo_root: Path) -> int:
    printable = " ".join(command)
    print(f"--- Running: {printable}")
    completed = subprocess.run(command, cwd=repo_root, check=False)
    return completed.returncode


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run tracer_windows_cli smoke flow (suite + runtime guard)."
    )
    parser.add_argument(
        "--build-dir",
        default="build_fast",
        help="Build directory name under apps/tracer_cli/windows (default: build_fast).",
    )
    parser.add_argument(
        "--no-concise",
        action="store_true",
        help="Disable concise output for suite execution.",
    )
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    concise_flags: list[str] = [] if args.no_concise else ["--concise"]

    suite_command = [
        sys.executable,
        "test/run.py",
        "--suite",
        "tracer_windows_cli",
        "--agent",
        "--build-dir",
        args.build_dir,
        *concise_flags,
    ]
    runtime_guard_command = [
        sys.executable,
        "test/runtime_guard.py",
        "--build-dir",
        args.build_dir,
    ]

    suite_exit = _run(suite_command, repo_root)
    if suite_exit != 0:
        print(f"[FAIL] tracer_windows_cli suite failed with exit={suite_exit}.")
        return suite_exit

    runtime_guard_exit = _run(runtime_guard_command, repo_root)
    if runtime_guard_exit != 0:
        print(f"[FAIL] runtime_guard failed with exit={runtime_guard_exit}.")
        return runtime_guard_exit

    print("[PASS] tracer_windows_cli smoke flow completed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

