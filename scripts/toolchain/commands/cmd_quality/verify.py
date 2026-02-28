import time
from typing import Literal

from ...core.context import Context
from ...core.executor import run_command
from ...services.suite_registry import (
    needs_suite_build,
    resolve_suite_bin_dir,
    resolve_suite_build_app,
    resolve_suite_name,
)
from ..cmd_build import BuildCommand
from . import verify_helpers


class VerifyCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    @staticmethod
    def _resolve_suite_config_override(
        suite_name: str,
        profile_name: str | None,
    ) -> str | None:
        return verify_helpers.resolve_suite_config_override(suite_name, profile_name)

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
        verify_helpers.write_build_only_result_json(
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
        suite_name = resolve_suite_name(app_name)
        build_app_name = resolve_suite_build_app(app_name) or app_name
        if build_app_name != app_name:
            print(f"--- verify: app `{app_name}` maps to suite build target `{build_app_name}`.")
        build_cmd = BuildCommand(self.ctx)
        build_ret = build_cmd.build(
            app_name=build_app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            kill_build_procs=kill_build_procs,
        )
        resolved_build_dir_name = build_cmd.resolve_build_dir_name(
            tidy=tidy,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            app_name=build_app_name,
        )
        if build_ret != 0:
            self._write_build_only_result_json(
                app_name=app_name,
                build_dir_name=resolved_build_dir_name,
                success=False,
                exit_code=build_ret,
                duration_seconds=time.monotonic() - started_at,
                error_message="Build failed during verify.",
                build_only=(suite_name is None),
            )
            return build_ret
        if suite_name is None and verify_scope != "unit":
            self._write_build_only_result_json(
                app_name=app_name,
                build_dir_name=resolved_build_dir_name,
                success=True,
                exit_code=0,
                duration_seconds=time.monotonic() - started_at,
            )
            return 0

        if verify_scope == "task":
            suite_ret = self.run_task_scope_checks(
                app_name=app_name,
                build_dir_name=resolved_build_dir_name,
            )
        elif verify_scope == "unit":
            suite_ret = self.run_unit_scope_checks()
        elif verify_scope == "artifact":
            suite_ret = self.run_artifact_scope_checks(
                app_name=app_name,
                build_dir_name=resolved_build_dir_name,
                profile_name=profile_name,
                concise=concise,
            )
        else:
            unit_ret = self.run_unit_scope_checks()
            if unit_ret != 0:
                suite_ret = unit_ret
            else:
                suite_ret = self.run_artifact_scope_checks(
                    app_name=app_name,
                    build_dir_name=resolved_build_dir_name,
                    profile_name=profile_name,
                    concise=concise,
                )
        if not suite_name:
            self._write_build_only_result_json(
                app_name=app_name,
                build_dir_name=resolved_build_dir_name,
                success=(suite_ret == 0),
                exit_code=suite_ret,
                duration_seconds=time.monotonic() - started_at,
                error_message=("Build-only verification failed." if suite_ret != 0 else ""),
            )
        return suite_ret

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
        return verify_helpers.run_internal_logic_tests(
            repo_root=self.ctx.repo_root,
            setup_env_fn=self.ctx.setup_env,
            run_command_fn=run_command,
        )

    def run_tests(
        self,
        app_name: str,
        build_dir_name: str,
        profile_name: str | None = None,
        concise: bool = False,
        skip_suite_build: bool = False,
    ) -> int:
        suite_name = resolve_suite_name(app_name)
        if not suite_name:
            print(
                f"--- verify: no mapped test suite for app `{app_name}`. "
                "Build-only verification completed."
            )
            return 0

        test_cmd = [
            "python",
            "test/run.py",
            "--suite",
            suite_name,
            "--agent",
        ]
        suite_bin_dir = resolve_suite_bin_dir(app_name)
        if suite_bin_dir:
            test_cmd.extend(["--bin-dir", suite_bin_dir])
        else:
            test_cmd.extend(["--build-dir", build_dir_name])
        suite_config_override = self._resolve_suite_config_override(
            suite_name=suite_name,
            profile_name=profile_name,
        )
        if suite_config_override:
            test_cmd.extend(["--config", suite_config_override])
        if needs_suite_build(app_name) and not skip_suite_build:
            test_cmd.extend(["--with-build", "--skip-configure"])
        if concise:
            test_cmd.append("--concise")

        suite_ret = run_command(
            test_cmd,
            cwd=self.ctx.repo_root,
            env=self.ctx.setup_env(),
        )
        if suite_ret != 0:
            return suite_ret

        markdown_gate_ret = verify_helpers.run_report_markdown_gates(
            repo_root=self.ctx.repo_root,
            setup_env_fn=self.ctx.setup_env,
            run_command_fn=run_command,
            app_name=app_name,
            build_dir_name=build_dir_name,
        )
        if markdown_gate_ret != 0:
            return markdown_gate_ret

        native_ret = self._run_native_core_runtime_tests(
            app_name=app_name,
            build_dir_name=build_dir_name,
        )
        if native_ret != 0:
            return native_ret
        return 0

    def run_task_scope_checks(self, app_name: str, build_dir_name: str) -> int:
        # Task-scope verify is intentionally lightweight and stable:
        # keep only native ABI/runtime smoke checks.
        return self._run_native_core_runtime_tests(
            app_name=app_name,
            build_dir_name=build_dir_name,
        )

    def _run_native_core_runtime_tests(self, app_name: str, build_dir_name: str) -> int:
        return verify_helpers.run_native_core_runtime_tests(
            repo_root=self.ctx.repo_root,
            setup_env_fn=self.ctx.setup_env,
            run_command_fn=run_command,
            app_name=app_name,
            build_dir_name=build_dir_name,
        )
