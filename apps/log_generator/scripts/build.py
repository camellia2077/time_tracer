#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

try:
    import tomllib as toml_lib
except ImportError:  # Python < 3.11
    try:
        import toml as toml_lib  # type: ignore
    except ImportError:  # pragma: no cover - runtime guard
        toml_lib = None


def resolve_compiler(name: str) -> tuple[str, str]:
    mapping = {
        "gcc": ("gcc", "g++"),
        "clang": ("clang", "clang++"),
    }
    if name not in mapping:
        raise ValueError(f"Unsupported compiler: {name}")
    cc, cxx = mapping[name]
    if shutil.which(cc) is None or shutil.which(cxx) is None:
        raise FileNotFoundError(f"Compiler not found in PATH: {cc} / {cxx}")
    return cc, cxx


def load_compiler_from_toml(path: Path) -> str | None:
    if not path.exists():
        return None
    if toml_lib is None:
        raise RuntimeError("TOML library not available. Install 'toml' or use Python 3.11+.")

    data = toml_lib.loads(path.read_text(encoding="utf-8"))
    toolchain = data.get("toolchain", {})
    compiler = toolchain.get("compiler")
    if compiler is None:
        return None
    compiler = str(compiler).strip().lower()
    return compiler if compiler else None


def run(cmd: list[str], env: dict[str, str] | None = None) -> None:
    print("+", " ".join(cmd))
    subprocess.check_call(cmd, env=env)


def main() -> int:
    parser = argparse.ArgumentParser(description="Configure and build log_generator.")
    parser.add_argument("--compiler", choices=["gcc", "clang"], help="Select compiler toolchain.")
    parser.add_argument("--config", default="scripts/build.toml",
                        help="TOML config path (default: scripts/build.toml).")
    parser.add_argument("--build-type", default="Release",
                        choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"])
    parser.add_argument("--optimize", dest="optimize", action="store_true", default=True,
                        help="Enable optimization flags (default).")
    parser.add_argument("--no-optimize", dest="optimize", action="store_false",
                        help="Disable optimization flags.")
    parser.add_argument("--tidy", dest="tidy", action="store_true", default=True,
                        help="Enable clang-tidy checks (default).")
    parser.add_argument("--no-tidy", dest="tidy", action="store_false",
                        help="Disable clang-tidy checks.")
    parser.add_argument("--build-dir", default="build", help="Build directory.")
    parser.add_argument("--generator", help="CMake generator name.")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    build_dir = root / args.build_dir

    env = os.environ.copy()
    config_path = root / args.config
    toml_compiler = load_compiler_from_toml(config_path)
    compiler_name = args.compiler or toml_compiler
    if compiler_name:
        cc, cxx = resolve_compiler(compiler_name)
        env["CC"] = cc
        env["CXX"] = cxx

    configure_cmd = [
        "cmake",
        "-S", str(root),
        "-B", str(build_dir),
        f"-DCMAKE_BUILD_TYPE={args.build_type}",
        f"-DENABLE_OPTIMIZATION={'ON' if args.optimize else 'OFF'}",
        f"-DENABLE_CLANG_TIDY={'ON' if args.tidy else 'OFF'}",
    ]
    if args.generator:
        configure_cmd.extend(["-G", args.generator])

    run(configure_cmd, env=env)
    run(["cmake", "--build", str(build_dir)], env=env)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
