from __future__ import annotations

import time

from ...core.context import Context
from ...services.change_policy import decide_post_change_policy
from ..cmd_build import BuildCommand
from ..cmd_quality.verify import VerifyCommand
from . import post_change_metrics, post_change_report, post_change_state


class PostChangeCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    @staticmethod
    def _elapsed_ms(started_at: float) -> int:
        return int((time.monotonic() - started_at) * 1000)

    @staticmethod
    def _print_metrics_summary(state: dict) -> None:
        metrics = state.get("metrics", {})
        timings = metrics.get("timings_ms", {})
        artifact_sizes = metrics.get("artifact_sizes", {})

        def format_duration(step_name: str) -> str:
            duration = timings.get(step_name)
            if duration is None:
                return "skipped"
            return f"{duration}ms"

        print(
            "--- post-change metrics: "
            f"configure={format_duration('configure')}, "
            f"build={format_duration('build')}, "
            f"test={format_duration('test')}, "
            f"total={timings.get('total', 0)}ms"
        )
        print(
            "--- post-change artifacts: "
            f"count={artifact_sizes.get('count', 0)}, "
            f"total_bytes={artifact_sizes.get('total_bytes', 0)}"
        )

    def execute(
        self,
        app_name: str | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
        run_tests: str = "auto",
        script_changes: str = "build",
        concise: bool = False,
        dry_run: bool = False,
        kill_build_procs: bool = False,
        cmake_args: list[str] | None = None,
    ) -> int:
        builder = BuildCommand(self.ctx)
        effective_cmake_args = [arg for arg in (cmake_args or []) if arg != "--"]
        resolved_build_dir_name = builder.resolve_build_dir_name(
            tidy=False,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            app_name=app_name,
        )
        decision = decide_post_change_policy(
            ctx=self.ctx,
            app_name=app_name,
            build_dir_name=resolved_build_dir_name,
            run_tests_mode=run_tests,
            script_changes_mode=script_changes,
        )
        decision.cmake_args = list(effective_cmake_args)
        if decision.cmake_args:
            decision.reasons.append(
                "Applied CLI --cmake-args passthrough to post-change configure/build flow."
            )
        if build_dir_name is None:
            decision.build_dir_name = builder.resolve_build_dir_name(
                tidy=False,
                build_dir_name=None,
                profile_name=profile_name,
                app_name=decision.app_name,
            )

        state_path = post_change_state.resolve_state_path(
            ctx=self.ctx,
            app_name=decision.app_name,
            build_dir_name=decision.build_dir_name,
        )
        state = post_change_state.new_state(
            decision=decision,
            state_path=state_path,
            profile_name=profile_name,
            run_tests=run_tests,
            script_changes=script_changes,
            concise=concise,
            dry_run=dry_run,
            kill_build_procs=kill_build_procs,
        )
        post_change_report.print_decision(decision)
        workflow_started_at = time.monotonic()

        def finalize_metrics(include_artifact_sizes: bool) -> None:
            post_change_state.set_total_duration_ms(
                state,
                self._elapsed_ms(workflow_started_at),
            )
            if include_artifact_sizes:
                post_change_state.set_artifact_sizes(
                    state,
                    post_change_metrics.collect_binary_size_metrics(
                        ctx=self.ctx,
                        app_name=decision.app_name,
                        build_dir_name=decision.build_dir_name,
                    ),
                )

        if dry_run:
            print("--- post-change: dry-run mode, no commands executed.")
            post_change_state.set_step(state, "configure", "skipped")
            post_change_state.set_step(state, "build", "skipped")
            post_change_state.set_step(state, "test", "skipped")
            post_change_state.set_total_duration_ms(state, 0)
            state["next_action"] = "Run without --dry-run to execute configure/build/test."
            return post_change_state.finish(
                state=state,
                state_path=state_path,
                exit_code=0,
                status="dry_run",
                repo_root=self.ctx.repo_root,
            )

        if not decision.need_configure and not decision.need_build and not decision.need_test:
            print("--- post-change: no build/test actions required.")
            post_change_state.set_step(state, "configure", "skipped")
            post_change_state.set_step(state, "build", "skipped")
            post_change_state.set_step(state, "test", "skipped")
            post_change_state.set_total_duration_ms(
                state,
                self._elapsed_ms(workflow_started_at),
            )
            state["next_action"] = "No action needed."
            return post_change_state.finish(
                state=state,
                state_path=state_path,
                exit_code=0,
                status="completed",
                repo_root=self.ctx.repo_root,
            )

        if decision.need_configure:
            post_change_state.set_step(state, "configure", "running")
            configure_started_at = time.monotonic()
            configure_ret = builder.configure(
                app_name=decision.app_name,
                tidy=False,
                extra_args=None,
                cmake_args=decision.cmake_args,
                build_dir_name=decision.build_dir_name,
                profile_name=profile_name,
                kill_build_procs=kill_build_procs,
            )
            post_change_state.set_step_duration_ms(
                state,
                "configure",
                self._elapsed_ms(configure_started_at),
            )
            if configure_ret != 0:
                finalize_metrics(include_artifact_sizes=True)
                self._print_metrics_summary(state)
                return post_change_state.finish_failure(
                    state=state,
                    state_path=state_path,
                    stage="configure",
                    exit_code=configure_ret,
                    repo_root=self.ctx.repo_root,
                )
            post_change_state.set_step(state, "configure", "done", configure_ret)
        else:
            post_change_state.set_step(state, "configure", "skipped")

        if decision.need_build:
            post_change_state.set_step(state, "build", "running")
            build_started_at = time.monotonic()
            build_ret = builder.build(
                app_name=decision.app_name,
                tidy=False,
                extra_args=None,
                cmake_args=None if decision.need_configure else decision.cmake_args,
                build_dir_name=decision.build_dir_name,
                profile_name=profile_name,
                kill_build_procs=kill_build_procs and not decision.need_configure,
            )
            post_change_state.set_step_duration_ms(
                state,
                "build",
                self._elapsed_ms(build_started_at),
            )
            if build_ret != 0:
                finalize_metrics(include_artifact_sizes=True)
                self._print_metrics_summary(state)
                return post_change_state.finish_failure(
                    state=state,
                    state_path=state_path,
                    stage="build",
                    exit_code=build_ret,
                    repo_root=self.ctx.repo_root,
                )
            post_change_state.set_step(state, "build", "done", build_ret)
        else:
            post_change_state.set_step(state, "build", "skipped")

        if decision.need_test:
            post_change_state.set_step(state, "test", "running")
            test_started_at = time.monotonic()
            test_ret = self._run_tests(
                app_name=decision.app_name,
                build_dir_name=decision.build_dir_name,
                concise=concise,
            )
            post_change_state.set_step_duration_ms(
                state,
                "test",
                self._elapsed_ms(test_started_at),
            )
            if test_ret != 0:
                finalize_metrics(include_artifact_sizes=True)
                self._print_metrics_summary(state)
                return post_change_state.finish_failure(
                    state=state,
                    state_path=state_path,
                    stage="test",
                    exit_code=test_ret,
                    repo_root=self.ctx.repo_root,
                )
            post_change_state.set_step(state, "test", "done", test_ret)
        else:
            post_change_state.set_step(state, "test", "skipped")

        finalize_metrics(include_artifact_sizes=True)
        self._print_metrics_summary(state)
        print("--- post-change: completed.")
        state["next_action"] = "Done."
        return post_change_state.finish(
            state=state,
            state_path=state_path,
            exit_code=0,
            status="completed",
            repo_root=self.ctx.repo_root,
        )

    def _run_tests(self, app_name: str, build_dir_name: str, concise: bool) -> int:
        return VerifyCommand(self.ctx).run_tests(
            app_name=app_name,
            build_dir_name=build_dir_name,
            concise=concise,
        )
