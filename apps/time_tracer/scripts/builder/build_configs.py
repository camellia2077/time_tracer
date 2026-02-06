# builder/build_configs.py

import tomllib
import sys
import argparse
from pathlib import Path
from typing import Dict, List, Tuple
from .ui.console import print_header, print_warning

class AppConfig:
    def __init__(self, config_file_path: Path):
        self._load(config_file_path)

    def _load(self, path: Path):
        if not path.exists():
            print(f"Error: Configuration '{path}' not found.")
            sys.exit(1)
            
        with open(path, "rb") as f:
            data = tomllib.load(f)
            
        build_settings = data.get('build', {})
        self.COMPILER = build_settings.get('compiler', 'default')
        self.WARNING_LEVEL = build_settings.get('warning_level', 2)
        self.ENABLE_LTO = build_settings.get('enable_lto', True)

def parse_arguments(args: List[str], default_compiler: str = 'default') -> Tuple[Dict, List[str]]:
    """
    Parse command line arguments for the build system.
    Supports subcommands: analyze, split, summary, clean, list, fix, auto.
    If no subcommand is present, defaults to 'build'.
    """
    print_header("Parsing command-line arguments...")

    # 0. Check for Subcommands
    command = "build"
    subcommand_args = []
    
    if args and args[0] in ["analyze", "a"]:
        return {"command": "analyze"}, []
    elif args and args[0] in ["split", "s"]:
        return {"command": "split"}, []
    elif args and args[0] in ["list", "l"]:
        return {"command": "list"}, []
    elif args and args[0] in ["fix", "f"]:
        # fix command might take args in future, currently standalone
        return {"command": "fix"}, []
    elif args and args[0] in ["auto", "aa"]:
        # auto command passes remaining args to the inner build
        return {"command": "auto", "extra_args": args[1:]}, []
    elif args and args[0] in ["summary", "sum"]:
        # summary takes task IDs
        return {"command": "summary", "task_ids": args[1:]}, []
    elif args and args[0] in ["clean", "c"]:
        # clean takes task IDs
        return {"command": "clean", "task_ids": args[1:]}, []

    # --- Default Build Command Parsing (Legacy) ---
    
    # 1. Base Logic (Install, Clean, Package)
    should_install = 'install' in args
    if should_install:
        args.remove('install')
        should_clean = True
        should_package = True
    else:
        should_clean = 'clean' in args
        should_package = '--package' in args or '-p' in args

    # 2. Compiler
    compiler = default_compiler
    if '--gcc' in args:
        compiler = 'gcc'; args.remove('--gcc')
    elif '--clang' in args:
        compiler = 'clang'; args.remove('--clang')

    # 3. Optimization & Features
    should_no_opt = '--no-opt' in args; (args.remove('--no-opt') if should_no_opt else None)
    should_no_tidy = '--no-tidy' in args; (args.remove('--no-tidy') if should_no_tidy else None)
    should_no_pch = '--no-pch' in args; (args.remove('--no-pch') if should_no_pch else None)
    should_no_lto = '--no-lto' in args; (args.remove('--no-lto') if should_no_lto else None)
    should_no_warn = '--no-warn' in args; (args.remove('--no-warn') if should_no_warn else None)

    # 4. Build Dir
    build_dir_name = None
    if '--build-dir' in args:
        try:
            idx = args.index('--build-dir')
            if idx + 1 < len(args):
                build_dir_name = args[idx + 1]
                args.pop(idx + 1); args.pop(idx)
        except ValueError:
            pass

    # 5. Tidy/Analysis Modes
    should_tidy = '--tidy' in args; (args.remove('--tidy') if should_tidy else None)
    should_fix = '--fix' in args; (args.remove('--fix') if should_fix else None)
    should_analyze_tidy = '--analyze-tidy' in args; (args.remove('--analyze-tidy') if should_analyze_tidy else None)
    should_fail_fast = '--fail-fast' in args; (args.remove('--fail-fast') if should_fail_fast else None)
    should_split_tasks = '--split-tasks' in args; (args.remove('--split-tasks') if should_split_tasks else None)
    
    # 5.1 Verify Fix Mode
    verify_fix_file = None
    if '--verify-fix' in args:
        try:
            idx = args.index('--verify-fix')
            if idx + 1 < len(args):
                verify_fix_file = args[idx + 1]
                args.pop(idx + 1); args.pop(idx)
        except ValueError:
            pass


    # 6. [NEW] Report Mode (AI-friendly errors)
    should_report = '--report' in args or '--report-mode' in args
    if should_report:
        if '--report' in args: args.remove('--report')
        if '--report-mode' in args: args.remove('--report-mode')
        print("--- Report mode: Concise error summary will be generated on failure.")

    # 6.1 Quiet Mode
    should_quiet = '--quiet' in args or '-q' in args
    if should_quiet:
        if '--quiet' in args: args.remove('--quiet')
        if '-q' in args: args.remove('-q')
        
    # 7. CMake Args (-D...)
    cmake_args = [arg for arg in args if arg.startswith('-D')]
    
    return {
        "command": command, # Default to "build"
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
        "quiet": should_quiet
    }, cmake_args
