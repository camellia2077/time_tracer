#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import subprocess
import sys
import time
from datetime import UTC, datetime
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Capture Windows build baseline for apps/tracer_core using the "
            "project Python toolchain entry."
        )
    )
    parser.add_argument(
        "--app",
        default="tracer_core",
        help="App name passed to scripts/run.py build (default: tracer_core).",
    )
    parser.add_argument(
        "--profile",
        default="release_safe",
        help="Build profile passed to scripts/run.py (default: release_safe).",
    )
    parser.add_argument(
        "--build-dir",
        default="build",
        help="Build directory name under apps/<app> (default: build).",
    )
    parser.add_argument(
        "--output",
        default="temp/windows_build_baseline.json",
        help="Output JSON path (relative to repo root by default).",
    )
    parser.add_argument(
        "--run-build",
        action="store_true",
        help="Run build via python scripts/run.py before collecting baseline.",
    )
    parser.add_argument(
        "--clean-first",
        action="store_true",
        help="Delete build directory before build (effective with --run-build).",
    )
    parser.add_argument(
        "--cmake-arg",
        action="append",
        default=[],
        metavar="ARG",
        help=("Extra CMake argument forwarded as --cmake-args=<ARG>. Can be repeated."),
    )
    return parser.parse_args()


def run_command(cmd: list[str], cwd: Path) -> tuple[int, float]:
    started = time.perf_counter()
    completed = subprocess.run(cmd, cwd=str(cwd), check=False)
    elapsed = round(time.perf_counter() - started, 3)
    return int(completed.returncode), elapsed


def read_cmake_generator(build_dir: Path) -> str:
    cache_path = build_dir / "CMakeCache.txt"
    if not cache_path.is_file():
        return ""

    for line in cache_path.read_text(encoding="utf-8", errors="ignore").splitlines():
        if line.startswith("CMAKE_GENERATOR:INTERNAL="):
            return line.split("=", 1)[1].strip()
    return ""


def collect_artifacts(bin_dir: Path) -> list[dict[str, object]]:
    artifacts: list[dict[str, object]] = []
    if not bin_dir.exists():
        return artifacts

    for file_path in sorted(p for p in bin_dir.iterdir() if p.is_file()):
        file_stat = file_path.stat()
        artifacts.append(
            {
                "scope": "bin",
                "name": file_path.name,
                "size_bytes": file_stat.st_size,
                "last_write_time": datetime.fromtimestamp(file_stat.st_mtime, tz=UTC).isoformat(),
            }
        )

    plugin_dir = bin_dir / "plugins"
    if plugin_dir.exists():
        for file_path in sorted(p for p in plugin_dir.iterdir() if p.is_file()):
            file_stat = file_path.stat()
            artifacts.append(
                {
                    "scope": "plugins",
                    "name": file_path.name,
                    "size_bytes": file_stat.st_size,
                    "last_write_time": datetime.fromtimestamp(
                        file_stat.st_mtime, tz=UTC
                    ).isoformat(),
                }
            )
    return artifacts


def parse_binary_imports(binary_path: Path) -> list[str]:
    objdump = "objdump"
    cmd = [objdump, "-p", str(binary_path)]
    try:
        completed = subprocess.run(
            cmd,
            check=False,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="ignore",
        )
    except FileNotFoundError:
        return []

    if completed.returncode != 0:
        return []

    imports: set[str] = set()
    for line in completed.stdout.splitlines():
        marker = "DLL Name:"
        if marker not in line:
            continue
        imports.add(line.split(marker, 1)[1].strip())
    return sorted(imports)


def collect_binary_imports(
    bin_dir: Path, artifacts: list[dict[str, object]]
) -> dict[str, list[str]]:
    imports: dict[str, list[str]] = {}
    for artifact in artifacts:
        name = str(artifact["name"])
        if not (name.endswith(".dll") or name.endswith(".exe")):
            continue

        if artifact["scope"] == "plugins":
            artifact_path = bin_dir / "plugins" / name
        else:
            artifact_path = bin_dir / name
        imports[name] = parse_binary_imports(artifact_path)
    return imports


def main() -> int:
    args = parse_args()
    repo_root = Path(__file__).resolve().parents[2]
    app_dir = repo_root / "apps" / args.app
    build_dir = app_dir / args.build_dir
    bin_dir = build_dir / "bin"
    output_path = Path(args.output)
    if not output_path.is_absolute():
        output_path = repo_root / output_path

    build_seconds: float | None = None
    build_exit_code: int | None = None
    if args.run_build:
        if args.clean_first and build_dir.exists():
            print(f"--- clean build dir: {build_dir}")
            # Keep cleanup logic in Python while build itself still uses scripts/run.py.
            import shutil

            shutil.rmtree(build_dir)

        build_cmd = [
            sys.executable,
            "scripts/run.py",
            "build",
            "--app",
            args.app,
            "--profile",
            args.profile,
            "--build-dir",
            args.build_dir,
        ]
        for cmake_arg in args.cmake_arg:
            build_cmd.append(f"--cmake-args={cmake_arg}")

        print("--- run build command:")
        print(" ".join(build_cmd))
        build_exit_code, build_seconds = run_command(build_cmd, repo_root)
        if build_exit_code != 0:
            print(f"Error: build failed with exit code {build_exit_code}.")
            return build_exit_code

    artifacts = collect_artifacts(bin_dir)
    binary_imports = collect_binary_imports(bin_dir, artifacts)

    runtime_dll_names = [
        "libgcc_s_seh-1.dll",
        "libstdc++-6.dll",
        "libwinpthread-1.dll",
        "libsqlite3-0.dll",
        "libtomlplusplus-3.dll",
    ]
    runtime_dll_presence = {
        dll_name: (bin_dir / dll_name).exists() for dll_name in runtime_dll_names
    }

    payload = {
        "timestamp_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "app": args.app,
        "build_profile": args.profile,
        "build_dir": str(build_dir),
        "bin_dir": str(bin_dir),
        "cmake_generator": read_cmake_generator(build_dir),
        "run_build": bool(args.run_build),
        "clean_first": bool(args.clean_first),
        "build_exit_code": build_exit_code,
        "build_duration_seconds": build_seconds,
        "runtime_dll_presence": runtime_dll_presence,
        "artifacts": artifacts,
        "binary_imports": binary_imports,
    }

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False), encoding="utf-8")
    print(f"Baseline written to {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
