# builder/core.py

import sys
import time
import os
import shutil
from pathlib import Path

# New modular imports
from . import build_configs, cmake_invoker, error_summarizer
from workflow import task_splitter
from .ui.console import print_header, print_error, print_success
from .ui.colors import AnsiColors
from .utils.logger import BuildLogger

class BuildPipeline:
    """Core controller for the physical build process."""
    
    def __init__(self, project_dir: Path, script_dir: Path):
        self.project_dir = project_dir
        self.script_dir = script_dir
        self.config = None
        self.options = {}
        self.logger = None

    def run(self, argv: list):
        start_time = time.monotonic()
        success = False
        command = "build"

        try:
            # 1. Load config
            config_path = self.script_dir / "config.toml"
            self.config = build_configs.AppConfig(config_path)

            # 2. Parse arguments
            self.options, cmake_args = build_configs.parse_arguments(argv, default_compiler=self.config.COMPILER)
            command = self.options.get("command", "build")

            if command == "build":
                # --- Standard Build Flow ---
                
                # 3. Prepare directory
                build_dir_name = self.options.get("build_dir") or "build"
                build_dir = self.project_dir / build_dir_name
                
                if self.options.get("clean") or self.options.get("install"):
                    if build_dir.exists():
                        shutil.rmtree(build_dir)
                build_dir.mkdir(exist_ok=True)
                os.chdir(build_dir)

                # 4. Logger
                self.logger = BuildLogger(log_path=Path("build.log"))

                # 5. Execute steps
                self._execute_steps(cmake_args)
                success = True
            
            else:
                # --- Refactor / Analysis Workflow ---
                from .refactor_orchestrator import RefactorOrchestrator
                orchestrator = RefactorOrchestrator(self.script_dir, self.project_dir)
                
                if command == "analyze":
                    orchestrator.run_analyze()
                elif command == "split":
                    orchestrator.run_split()
                elif command == "list":
                    orchestrator.list_tasks()
                elif command == "clean":
                    orchestrator.run_clean(self.options.get("task_ids", []))
                elif command == "summary":
                    orchestrator.run_summary(self.options.get("task_ids", []))
                elif command == "fix":
                    orchestrator.run_fix()
                elif command == "auto":
                    orchestrator.run_auto(self.options.get("extra_args", []))
                
                success = True

        except Exception as e:
            print_error(f"\nPipeline Failed: {e}")
            if self.logger:
                self.logger.log_error(f"Build Failed: {e}")
            
            # AI-friendly error report (Only for build command really, but safe to leave)
            if self.options.get("report"):
                 log_path = Path("build.log")
                 if log_path.exists():
                     lines = log_path.read_text(encoding="utf-8", errors="ignore").splitlines()
                     errors = error_summarizer.extract_errors(lines)
                     error_summarizer.print_error_summary(errors)

        finally:
            # Only print build summary for actual builds
            if command == "build":
                self._print_summary(start_time, self.options.get("build_dir") or "build", success)
        
        if not success:
            sys.exit(1)

    def _execute_steps(self, cmake_args):
        # 0. Verify Fix Pre-step (Touch file)
        if self.options.get("verify_fix_file"):
            target_file = Path(self.options["verify_fix_file"])
            if target_file.exists():
                print(f"--- [Verify-Fix] Touching file to force rebuild: {target_file}")
                # Update modification time to now
                target_file.touch()
            else:
                 print_error(f"--- [Verify-Fix] Error: File not found: {target_file}")

        # Configure
        cmake_invoker.run_cmake(
            self.options["package"], cmake_args, self.options["compiler"], self.config, self.logger,
            no_opt=self.options.get("no_opt"), no_tidy=self.options.get("no_tidy"),
            no_lto=self.options.get("no_lto"), no_warn=self.options.get("no_warn"),
            no_pch=self.options.get("no_pch"), fail_fast=self.options.get("fail_fast"),
            quiet=self.options.get("quiet")
        )
        
        # Build
        target = ("tidy-fix" if self.options.get("fix") else "tidy") if self.options.get("tidy") else None
        build_log, build_success = cmake_invoker.run_build(logger=self.logger, target=target, quiet=self.options.get("quiet"))

        # Split tidy logs into task files if requested (do this even if build failed)
        if self.options.get("split_tasks") and target == "tidy" and build_log:
            tasks_dir = Path.cwd() / "tasks"
            count = task_splitter.split_tidy_logs(build_log, tasks_dir)
            print(f"--- Created {count} task log files in {tasks_dir}")

        # Raise exception if build failed (after task splitting is done)
        if not build_success:
            import subprocess
            raise subprocess.CalledProcessError(1, "ninja")

        # CPack & Install
        if self.options["package"]:
            installer = cmake_invoker.run_cpack(logger=self.logger)
            if self.options["install"]:
                 print_header("Launching the installer...")
                 import subprocess
                 subprocess.run([installer], check=True)

    def _print_summary(self, start_time, build_dir, success):
        duration = int(time.monotonic() - start_time)
        m, s = divmod(duration, 60)
        print("\n" + "="*60)
        if success:
            print_success(f"{AnsiColors.BOLD}Build SUCCESS!{AnsiColors.ENDC}")
            print(f"Artifacts: {build_dir}")
        else:
            print_error(f"{AnsiColors.BOLD}Building FAILED!{AnsiColors.ENDC}")
            print("Check build.log for details.")
        print("-" * 60)
        print(f"Total time elapsed: {AnsiColors.BOLD}{m}m {s}s{AnsiColors.ENDC}")
        print("="*60)
