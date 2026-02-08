#!/usr/bin/env python3
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def _run(cmd: list[str]) -> int:
    print("+", " ".join(cmd))
    return subprocess.call(cmd)


def _configure_with_preset(preset: str, cmake_args: list[str]) -> int:
    return _run(["cmake", "--preset", preset, *cmake_args])


def _build_with_preset(preset: str, target: str | None) -> int:
    cmd = ["cmake", "--build", "--preset", preset]
    if target:
        cmd.extend(["--target", target])
    return _run(cmd)


def _configure(build_dir: Path, root: Path, cmake_args: list[str]) -> int:
    return _run(["cmake", "-S", str(root), "-B", str(build_dir), *cmake_args])


def _build(build_dir: Path, target: str | None) -> int:
    cmd = ["cmake", "--build", str(build_dir)]
    if target:
        cmd.extend(["--target", target])
    return _run(cmd)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Build entrypoint for log_generator.")
    parser.add_argument(
        "command",
        nargs="?",
        default="build",
        choices=["build", "fast", "tidy", "tidy-fix", "format"],
        help="Build command to run.",
    )
    parser.add_argument("--preset", help="CMake preset name.")
    parser.add_argument("--build-dir", help="Build directory.")
    parser.add_argument("--target", help="Override build target.")
    parser.add_argument(
        "--no-tidy",
        action="store_true",
        help="Disable clang-tidy during configuration.",
    )
    args, extra = parser.parse_known_args(argv)

    if extra:
        print(f"Warning: ignoring extra args: {' '.join(extra)}")

    cmake_args: list[str] = []
    if args.no_tidy:
        cmake_args.append("-DENABLE_CLANG_TIDY=OFF")

    script_dir = Path(__file__).resolve().parent
    root = script_dir.parent

    default_build_dir = {
        "build": "build",
        "fast": "build_fast",
        "tidy": "build_tidy",
        "tidy-fix": "build_tidy",
        "format": "build",
    }[args.command]

    default_target = {
        "build": None,
        "fast": None,
        "tidy": "tidy",
        "tidy-fix": "tidy-fix",
        "format": "format",
    }[args.command]

    target = args.target or default_target

    if args.preset:
        code = _configure_with_preset(args.preset, cmake_args)
        if code != 0:
            return code
        return _build_with_preset(args.preset, target)

    build_dir = Path(args.build_dir or default_build_dir)
    if not build_dir.is_absolute():
        build_dir = root / build_dir

    if args.no_tidy:
        extra.append("-DENABLE_CLANG_TIDY=OFF")
    code = _configure(build_dir, root, cmake_args)
    if code != 0:
        return code
    return _build(build_dir, target)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
