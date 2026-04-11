import time

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
from ..shared.result_reporting import print_verify_phase_summary
from .verify_internal.verify_build_stage import execute_build_stage, handle_post_build_state
from .verify_internal.verify_command_text import build_verify_command_text
from .verify_internal.verify_markdown_gate_runner import run_report_markdown_gates
from .verify_internal.verify_native_runner import run_native_core_runtime_tests
from .verify_internal.verify_pipeline import run_artifact_pipeline
from .verify_internal.verify_profile_inference import infer_verify_profiles
from .verify_internal.verify_profile_policy import resolve_suite_config_override
from .verify_internal.verify_result_writer import write_build_only_result_json
from .verify_internal.verify_result_writer import merge_verify_phase_summary_into_result_json
from .verify_internal.verify_suite_runner import build_suite_test_command
from .self_test import SelfTestCommand


class VerifyCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    @staticmethod
    def _build_verify_command_text(
        *,
        app_name: str,
        profile_name: str | None,
        build_dir_name: str | None,
        concise: bool,
        tidy: bool,
        kill_build_procs: bool,
        cmake_args: list[str] | None,
    ) -> str:
        return build_verify_command_text(
            app_name=app_name,
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
        verify_phases: list[dict[str, object]] | None = None,
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
            verify_phases=verify_phases,
        )

    @staticmethod
    def _record_verify_phase(
        verify_phases: list[dict[str, object]],
        *,
        name: str,
        category: str,
        status: str,
        exit_code: int,
    ) -> None:
        verify_phases.append(
            {
                "name": name,
                "category": category,
                "status": status,
                "exit_code": int(exit_code),
            }
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
        run_command_fn=None,
    ) -> int:
        if profile_name or app_name not in {"tracer_core", "tracer_core_shell"}:
            return self._execute_single(
                app_name=app_name,
                tidy=tidy,
                extra_args=extra_args,
                cmake_args=cmake_args,
                build_dir_name=build_dir_name,
                profile_name=profile_name,
                concise=concise,
                kill_build_procs=kill_build_procs,
                run_command_fn=run_command_fn,
                skip_unit_checks=False,
            )

        inference = infer_verify_profiles(self.ctx.repo_root)
        if inference.fallback_to_fast:
            print(f"--- verify: falling back to profile `fast`: {inference.reason}")
            return self._execute_single(
                app_name=app_name,
                tidy=tidy,
                extra_args=extra_args,
                cmake_args=cmake_args,
                build_dir_name=build_dir_name,
                profile_name="fast",
                concise=concise,
                kill_build_procs=kill_build_procs,
                run_command_fn=run_command_fn,
                skip_unit_checks=False,
            )

        if len(inference.profiles) == 1:
            print(f"--- verify: inferred focused profile `{inference.profiles[0]}`")
            return self._execute_single(
                app_name=app_name,
                tidy=tidy,
                extra_args=extra_args,
                cmake_args=cmake_args,
                build_dir_name=build_dir_name,
                profile_name=inference.profiles[0],
                concise=concise,
                kill_build_procs=kill_build_procs,
                run_command_fn=run_command_fn,
                skip_unit_checks=False,
            )

        effective_run_command = run_command if run_command_fn is None else run_command_fn
        inferred_profiles = ", ".join(inference.profiles)
        print(f"--- verify: inferred focused profiles: {inferred_profiles}")
        unit_ret = self.run_unit_scope_checks(run_command_fn=effective_run_command)
        if unit_ret != 0:
            self._report_verify_failure(
                app_name=app_name,
                command_text=self._build_verify_command_text(
                    app_name=app_name,
                    profile_name=None,
                    build_dir_name=build_dir_name,
                    concise=concise,
                    tidy=tidy,
                    kill_build_procs=kill_build_procs,
                    cmake_args=cmake_args,
                ),
                exit_code=unit_ret,
                stage="verify-unit",
            )
            print_result_paths(app_name=app_name, repo_root=self.ctx.repo_root)
            return unit_ret

        for index, inferred_profile in enumerate(inference.profiles, start=1):
            print(
                f"--- verify: running inferred profile [{index}/{len(inference.profiles)}] "
                f"`{inferred_profile}`"
            )
            ret = self._execute_single(
                app_name=app_name,
                tidy=tidy,
                extra_args=extra_args,
                cmake_args=cmake_args,
                build_dir_name=build_dir_name,
                profile_name=inferred_profile,
                concise=concise,
                kill_build_procs=kill_build_procs,
                run_command_fn=run_command_fn,
                skip_unit_checks=True,
            )
            if ret != 0:
                return ret
        return 0

    def _execute_single(
        self,
        *,
        app_name: str,
        tidy: bool = False,
        extra_args: list[str] | None = None,
        cmake_args: list[str] | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
        run_command_fn=None,
        skip_unit_checks: bool = False,
    ) -> int:
        started_at = time.monotonic()
        verify_phases: list[dict[str, object]] = []
        effective_run_command = run_command if run_command_fn is None else run_command_fn
        verify_command_text = self._build_verify_command_text(
            app_name=app_name,
            profile_name=profile_name,
            build_dir_name=build_dir_name,
            concise=concise,
            tidy=tidy,
            kill_build_procs=kill_build_procs,
            cmake_args=cmake_args,
        )
        suite_name = resolve_suite_name(app_name)
        build_ret, resolved_build_dir_name, build_app_name, build_log_path = execute_build_stage(
            ctx=self.ctx,
            build_command_cls=BuildCommand,
            app_name=app_name,
            tidy=tidy,
            extra_args=extra_args,
            cmake_args=cmake_args,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            concise=concise,
            kill_build_procs=kill_build_procs,
            run_command_fn=effective_run_command,
        )
        if build_app_name != app_name:
            print(f"--- verify: app `{app_name}` maps to suite build target `{build_app_name}`.")
        self._record_verify_phase(
            verify_phases,
            name="build",
            category="verify",
            status="passed" if build_ret == 0 else "failed",
            exit_code=build_ret,
        )

        early_exit = handle_post_build_state(
            suite_name=suite_name,
            build_ret=build_ret,
            app_name=app_name,
            resolved_build_dir_name=resolved_build_dir_name,
            started_at=started_at,
            verify_command_text=verify_command_text,
            repo_root=self.ctx.repo_root,
            write_build_only_result_json_fn=self._write_build_only_result_json,
            print_failure_report_fn=print_failure_report,
            print_result_paths_fn=print_result_paths,
            build_log_path=build_log_path,
            verify_phases=verify_phases,
        )
        if early_exit is not None:
            return int(early_exit)

        if not skip_unit_checks:
            unit_ret = self.run_unit_scope_checks(
                run_command_fn=effective_run_command,
            )
            if unit_ret != 0:
                self._record_verify_phase(
                    verify_phases,
                    name="verify_unit",
                    category="verify",
                    status="failed",
                    exit_code=unit_ret,
                )
                if not suite_name:
                    self._write_build_only_result_json(
                        app_name=app_name,
                        build_dir_name=resolved_build_dir_name,
                        success=False,
                        exit_code=unit_ret,
                        duration_seconds=time.monotonic() - started_at,
                        error_message="Unit verification failed.",
                        build_only=True,
                        verify_phases=verify_phases,
                    )
                self._report_verify_failure(
                    app_name=app_name,
                    command_text=verify_command_text,
                    exit_code=unit_ret,
                    stage="verify-unit",
                )
                print_result_paths(app_name=app_name, repo_root=self.ctx.repo_root)
                return unit_ret
            self._record_verify_phase(
                verify_phases,
                name="verify_unit",
                category="verify",
                status="passed",
                exit_code=0,
            )

        artifact_ret = self.run_artifact_scope_checks(
            app_name=app_name,
            build_dir_name=resolved_build_dir_name,
            profile_name=profile_name,
            concise=concise,
            run_command_fn=effective_run_command,
            verify_phases=verify_phases,
        )
        if not suite_name:
            self._write_build_only_result_json(
                app_name=app_name,
                build_dir_name=resolved_build_dir_name,
                success=(artifact_ret == 0),
                exit_code=artifact_ret,
                duration_seconds=time.monotonic() - started_at,
                error_message=(
                    "Full verification failed." if artifact_ret != 0 else ""
                ),
                build_only=True,
                verify_phases=verify_phases,
            )
        else:
            merge_verify_phase_summary_into_result_json(
                repo_root=self.ctx.repo_root,
                app_name=app_name,
                verify_phases=verify_phases,
            )

        if artifact_ret != 0:
            self._report_verify_failure(
                app_name=app_name,
                command_text=verify_command_text,
                exit_code=artifact_ret,
                stage="verify-artifact",
            )

        print_verify_phase_summary(verify_phases)
        print_result_paths(app_name=app_name, repo_root=self.ctx.repo_root)
        return artifact_ret

    def _report_verify_failure(
        self,
        *,
        app_name: str,
        command_text: str,
        exit_code: int,
        stage: str,
    ) -> None:
        print_failure_report(
            command=command_text,
            exit_code=exit_code,
            next_action=f"Fix errors and rerun: {command_text}",
            app_name=app_name,
            repo_root=self.ctx.repo_root,
            stage=stage,
            fallback_key_error_hint="Verification failed. See command output above.",
        )

    def run_artifact_scope_checks(
        self,
        app_name: str,
        build_dir_name: str,
        profile_name: str | None = None,
        concise: bool = False,
        run_command_fn=None,
        verify_phases: list[dict[str, object]] | None = None,
        ) -> int:
        return self.run_tests(
            app_name=app_name,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            concise=concise,
            skip_suite_build=True,
            run_command_fn=run_command if run_command_fn is None else run_command_fn,
            verify_phases=verify_phases,
        )

    def run_unit_scope_checks(self, run_command_fn=None) -> int:
        print("--- verify: running internal logic tests (Python unit/component)")
        return SelfTestCommand(self.ctx).execute(
            verbose=False,
            group="verify-stack",
            run_command_fn=run_command if run_command_fn is None else run_command_fn,
        )

    def run_tests(
        self,
        app_name: str,
        build_dir_name: str,
        profile_name: str | None = None,
        concise: bool = False,
        skip_suite_build: bool = False,
        run_command_fn=None,
        verify_phases: list[dict[str, object]] | None = None,
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
        effective_run_command = run_command if run_command_fn is None else run_command_fn
        return run_artifact_pipeline(
            test_cmd=test_cmd,
            app_name=app_name,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            repo_root=self.ctx.repo_root,
            setup_env_fn=self.ctx.setup_env,
            run_command_fn=effective_run_command,
            run_report_markdown_gates_fn=lambda **kwargs: run_report_markdown_gates(
                **kwargs,
                normalize_ext=tuple(self.ctx.config.quality.gate_audit.normalize_ext),
            ),
            run_native_core_runtime_tests_fn=run_native_core_runtime_tests,
            record_phase_fn=(
                None
                if verify_phases is None
                else lambda **kwargs: self._record_verify_phase(verify_phases, **kwargs)
            ),
        )
