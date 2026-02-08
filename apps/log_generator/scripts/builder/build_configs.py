# builder/build_configs.py

import argparse
import sys
import tomllib
from pathlib import Path
from typing import Dict, List, Tuple

from .ui.console import print_header


class AppConfig:
    def __init__(self, config_file_path: Path):
        self._load(config_file_path)

    def _load(self, path: Path):
        if not path.exists():
            print(f"Error: Configuration '{path}' not found.")
            sys.exit(1)

        with open(path, "rb") as handle:
            data = tomllib.load(handle)

        toolchain = data.get("toolchain", {})
        build_settings = data.get("build", {})

        compiler = toolchain.get("compiler") or build_settings.get("compiler", "default")
        self.COMPILER = str(compiler).strip().lower() if compiler else "default"
        self.WARNING_LEVEL = build_settings.get("warning_level", 2)
        self.ENABLE_LTO = build_settings.get("enable_lto", True)


def parse_arguments(args: List[str], default_compiler: str = "default") -> Tuple[Dict, List[str]]:
    """
    Parse command line arguments for the build system.
    Supports subcommands: analyze, split, summary, clean, list, fix, auto.
    If no subcommand is present, defaults to 'build'.
    """
    print_header("Parsing command-line arguments...")

    command = "build"

    if args and args[0] in ["analyze", "a"]:
        return {"command": "analyze"}, []
    if args and args[0] in ["split", "s"]:
        return {"command": "split"}, []
    if args and args[0] in ["list", "l"]:
        return {"command": "list"}, []
    if args and args[0] in ["fix", "f"]:
        return {"command": "fix"}, []
    if args and args[0] in ["auto", "aa"]:
        return {"command": "auto", "extra_args": args[1:]}, []
    if args and args[0] in ["summary", "sum"]:
        return {"command": "summary", "task_ids": args[1:]}, []
    if args and args[0] in ["clean", "c"]:
        return {"command": "clean", "task_ids": args[1:]}, []

    should_install = "install" in args
    if should_install:
        args.remove("install")
        should_clean = True
        should_package = True
    else:
        should_clean = "clean" in args
        should_package = "--package" in args or "-p" in args

    compiler = default_compiler
    if "--gcc" in args:
        compiler = "gcc"
        args.remove("--gcc")
    elif "--clang" in args:
        compiler = "clang"
        args.remove("--clang")

    should_no_opt = "--no-opt" in args
    if should_no_opt:
        args.remove("--no-opt")
    should_no_tidy = "--no-tidy" in args
    if should_no_tidy:
        args.remove("--no-tidy")
    should_no_pch = "--no-pch" in args
    if should_no_pch:
        args.remove("--no-pch")
    should_no_lto = "--no-lto" in args
    if should_no_lto:
        args.remove("--no-lto")
    should_no_warn = "--no-warn" in args
    if should_no_warn:
        args.remove("--no-warn")

    build_dir_name = None
    if "--build-dir" in args:
        try:
            idx = args.index("--build-dir")
            if idx + 1 < len(args):
                build_dir_name = args[idx + 1]
                args.pop(idx + 1)
                args.pop(idx)
        except ValueError:
            pass

    should_tidy = "--tidy" in args
    if should_tidy:
        args.remove("--tidy")
    should_fix = "--fix" in args
    if should_fix:
        args.remove("--fix")
    should_analyze_tidy = "--analyze-tidy" in args
    if should_analyze_tidy:
        args.remove("--analyze-tidy")
    should_fail_fast = "--fail-fast" in args
    if should_fail_fast:
        args.remove("--fail-fast")
    should_split_tasks = "--split-tasks" in args
    if should_split_tasks:
        args.remove("--split-tasks")

    verify_fix_file = None
    if "--verify-fix" in args:
        try:
            idx = args.index("--verify-fix")
            if idx + 1 < len(args):
                verify_fix_file = args[idx + 1]
                args.pop(idx + 1)
                args.pop(idx)
        except ValueError:
            pass

    should_report = "--report" in args or "--report-mode" in args
    if should_report:
        if "--report" in args:
            args.remove("--report")
        if "--report-mode" in args:
            args.remove("--report-mode")
        print("--- Report mode: Concise error summary will be generated on failure.")

    should_quiet = "--quiet" in args or "-q" in args
    if should_quiet:
        if "--quiet" in args:
            args.remove("--quiet")
        if "-q" in args:
            args.remove("-q")

    cmake_args = [arg for arg in args if arg.startswith("-D")]

    return {
        "command": command,
        "clean": should_clean,
        "package": should_package,
        "install": should_install,
        "compiler": compiler,
        "no_opt": should_no_opt,
        "no_tidy": should_no_tidy,
        "no_lto": should_no_lto,
        "no_warn": should_no_warn,
        "no_pch": should_no_pch,
        "build_dir": build_dir_name,
        "tidy": should_tidy,
        "analyze_tidy": should_analyze_tidy,
        "fail_fast": should_fail_fast,
        "fix": should_fix,
        "split_tasks": should_split_tasks,
        "verify_fix_file": verify_fix_file,
        "report": should_report,
        "quiet": should_quiet,
    }, cmake_args
