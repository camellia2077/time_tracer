import sys
import time
from typing import Literal

from ...core.context import Context
from ...core.executor import run_command
from ...services.suite_registry import (
    needs_suite_build,
    resolve_suite_bin_dir,
    resolve_suite_name,
    resolve_suite_runner_name,
)
from ..cmd_build import BuildCommand
from ..shared.result_reporting import print_failure_report, print_result_paths
from .verify_internal.verify_build_stage import execute_build_stage, handle_post_build_state
from .verify_internal.verify_command_text import build_verify_command_text
from .verify_internal.verify_markdown_gate_runner import run_report_markdown_gates
from .verify_internal.verify_native_runner import run_native_core_runtime_tests
from .verify_internal.verify_pipeline import run_artifact_pipeline, run_scope_pipeline
from .verify_internal.verify_profile_policy import resolve_suite_config_override
from .verify_internal.verify_result_writer import write_build_only_result_json
from .verify_internal.verify_suite_runner import build_suite_test_command


class VerifyCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    @staticmethod
    def _build_verify_command_text(
        *,
        app_name: str,
        verify_scope: str,
        profile_name: str | None,
        build_dir_name: str | None,
        concise: bool,
        tidy: bool,
        kill_build_procs: bool,
        cmake_args: list[str] | None,
    ) -> str:
        return build_verify_command_text(
            app_name=app_name,
            verify_scope=verify_scope,
            profile_name=profile_name,
            build_dir_name=build_dir_name,
            concise=concise,
            tidy=tidy,
            kill_build_procs=kill_build_procs,
            cmake_args=cmake_args,
        )

    @staticmethod
    def _resolve_suite_config_override(
        suite_name: str,
        profile_name: str | None,
    ) -> str | None:
        return resolve_suite_config_override(suite_name, profile_name)

    def _write_build_only_result_json(
        self,
        app_name: str,
        build_dir_name: str,
        success: bool,
        exit_code: int,
        duration_seconds: float,
        error_message: str = "",
        build_only: bool = True,
    ) -> None:
        write_build_only_result_json(
            repo_root=self.ctx.repo_root,
            app_name=app_name,
            build_dir_name=build_dir_name,
            success=success,
            exit_code=exit_code,
            duration_seconds=duration_seconds,
            error_message=error_message,
            build_only=build_only,
        )

    def execute(
        self,
        app_name: str,
        tidy: bool = False,
        extra_args: list[str] | None = None,
        cmake_args: list[str] | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
        verify_scope: Literal["task", "unit", "artifact", "batch"] = "batch",
    ) -> int:
        started_at = time.monotonic()
        verify_command_text = self._build_verify_command_text(
            app_name=app_name,
            verify_scope=verify_scope,
            profile_name=profile_name,
            build_dir_name=build_dir_name,
            concise=concise,
            tidy=tidy,
            kill_build_procs=kill_build_procs,
            cmake_args=cmake_args,
        )
        suite_name = resolve_suite_name(app_name)
        build_ret, resolved_build_dir_name, build_app_name = execute_build_stage(
            ctx=self.ctx,
            build_command_cls=BuildCommand,
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            kill_build_procs=kill_build_procs,
        )
        if build_app_name != app_name:
            print(f"--- verify: app `{app_name}` maps to suite build target `{build_app_name}`.")

        early_exit = handle_post_build_state(
            suite_name=suite_name,
            verify_scope=verify_scope,
            build_ret=build_ret,
            app_name=app_name,
            resolved_build_dir_name=resolved_build_dir_name,
            started_at=started_at,
            verify_command_text=verify_command_text,
            repo_root=self.ctx.repo_root,
            write_build_only_result_json_fn=self._write_build_only_result_json,
            print_failure_report_fn=print_failure_report,
            print_result_paths_fn=print_result_paths,
        )
        if early_exit is not None:
            return int(early_exit)

        return run_scope_pipeline(
            verify_scope=verify_scope,
            run_task_scope_checks=lambda: self.run_task_scope_checks(
                app_name=app_name,
                build_dir_name=resolved_build_dir_name,
            ),
            run_unit_scope_checks=self.run_unit_scope_checks,
            run_artifact_scope_checks=lambda: self.run_artifact_scope_checks(
                app_name=app_name,
                build_dir_name=resolved_build_dir_name,
                profile_name=profile_name,
                concise=concise,
            ),
            suite_name=suite_name,
            app_name=app_name,
            resolved_build_dir_name=resolved_build_dir_name,
            started_at=started_at,
            verify_command_text=verify_command_text,
            repo_root=self.ctx.repo_root,
            write_build_only_result_json_fn=self._write_build_only_result_json,
            print_failure_report_fn=print_failure_report,
            print_result_paths_fn=print_result_paths,
        )

    def run_artifact_scope_checks(
        self,
        app_name: str,
        build_dir_name: str,
        profile_name: str | None = None,
        concise: bool = False,
    ) -> int:
        return self.run_tests(
            app_name=app_name,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            concise=concise,
            skip_suite_build=True,
        )

    def run_unit_scope_checks(self) -> int:
        test_cmd = [
            sys.executable,
            "-m",
            "unittest",
            "scripts.tests.build.test_build_profiles_cmake",
            "scripts.tests.build.test_build_profiles_gradle",
            "scripts.tests.build.test_build_profiles_cargo",
            "scripts.tests.verify.test_verify_run_tests",
            "scripts.tests.verify.test_verify_execute_flow",
            "scripts.tests.verify.test_verify_cli_handler",
        ]
        print("--- verify: running internal logic tests (Python unit/component)")
        return run_command(
            test_cmd,
            cwd=self.ctx.repo_root,
            env=self.ctx.setup_env(),
        )

    def run_tests(
        self,
        app_name: str,
        build_dir_name: str,
        profile_name: str | None = None,
        concise: bool = False,
        skip_suite_build: bool = False,
    ) -> int:
        test_cmd = build_suite_test_command(
            app_name=app_name,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            concise=concise,
            skip_suite_build=skip_suite_build,
            resolve_suite_name_fn=resolve_suite_name,
            resolve_suite_runner_name_fn=resolve_suite_runner_name,
            resolve_suite_bin_dir_fn=resolve_suite_bin_dir,
            needs_suite_build_fn=needs_suite_build,
            resolve_suite_config_override_fn=self._resolve_suite_config_override,
        )
        return run_artifact_pipeline(
            test_cmd=test_cmd,
            app_name=app_name,
            build_dir_name=build_dir_name,
            repo_root=self.ctx.repo_root,
            setup_env_fn=self.ctx.setup_env,
            run_command_fn=run_command,
            run_report_markdown_gates_fn=run_report_markdown_gates,
            run_native_core_runtime_tests_fn=run_native_core_runtime_tests,
        )

    def run_task_scope_checks(self, app_name: str, build_dir_name: str) -> int:
        # Task-scope verify is intentionally lightweight and stable:
        # keep only native ABI/runtime smoke checks.
        return self._run_native_core_runtime_tests(
            app_name=app_name,
            build_dir_name=build_dir_name,
        )

    def _run_native_core_runtime_tests(self, app_name: str, build_dir_name: str) -> int:
        return run_native_core_runtime_tests(
            repo_root=self.ctx.repo_root,
            setup_env_fn=self.ctx.setup_env,
            run_command_fn=run_command,
            app_name=app_name,
            build_dir_name=build_dir_name,
        )
