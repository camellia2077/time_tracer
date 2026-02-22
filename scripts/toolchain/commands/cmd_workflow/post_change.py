from __future__ import annotations

from ...core.context import Context
from ...services.change_policy import decide_post_change_policy
from ..cmd_build import BuildCommand
from ..cmd_quality.verify import VerifyCommand
from . import post_change_report, post_change_state


class PostChangeCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

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
    ) -> int:
        builder = BuildCommand(self.ctx)
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

        if dry_run:
            print("--- post-change: dry-run mode, no commands executed.")
            post_change_state.set_step(state, "configure", "skipped")
            post_change_state.set_step(state, "build", "skipped")
            post_change_state.set_step(state, "test", "skipped")
            state["next_action"] = "Run without --dry-run to execute configure/build/test."
            return post_change_state.finish(
                state=state,
                state_path=state_path,
                exit_code=0,
                status="dry_run",
            )

        if not decision.need_configure and not decision.need_build and not decision.need_test:
            print("--- post-change: no build/test actions required.")
            post_change_state.set_step(state, "configure", "skipped")
            post_change_state.set_step(state, "build", "skipped")
            post_change_state.set_step(state, "test", "skipped")
            state["next_action"] = "No action needed."
            return post_change_state.finish(
                state=state,
                state_path=state_path,
                exit_code=0,
                status="completed",
            )

        if decision.need_configure:
            post_change_state.set_step(state, "configure", "running")
            configure_ret = builder.configure(
                app_name=decision.app_name,
                tidy=False,
                extra_args=None,
                cmake_args=decision.cmake_args,
                build_dir_name=decision.build_dir_name,
                profile_name=profile_name,
                kill_build_procs=kill_build_procs,
            )
            if configure_ret != 0:
                return post_change_state.finish_failure(
                    state=state,
                    state_path=state_path,
                    stage="configure",
                    exit_code=configure_ret,
                )
            post_change_state.set_step(state, "configure", "done", configure_ret)
        else:
            post_change_state.set_step(state, "configure", "skipped")

        if decision.need_build:
            post_change_state.set_step(state, "build", "running")
            build_ret = builder.build(
                app_name=decision.app_name,
                tidy=False,
                extra_args=None,
                cmake_args=None if decision.need_configure else decision.cmake_args,
                build_dir_name=decision.build_dir_name,
                profile_name=profile_name,
                kill_build_procs=kill_build_procs and not decision.need_configure,
            )
            if build_ret != 0:
                return post_change_state.finish_failure(
                    state=state,
                    state_path=state_path,
                    stage="build",
                    exit_code=build_ret,
                )
            post_change_state.set_step(state, "build", "done", build_ret)
        else:
            post_change_state.set_step(state, "build", "skipped")

        if decision.need_test:
            post_change_state.set_step(state, "test", "running")
            test_ret = self._run_tests(
                app_name=decision.app_name,
                build_dir_name=decision.build_dir_name,
                concise=concise,
            )
            if test_ret != 0:
                return post_change_state.finish_failure(
                    state=state,
                    state_path=state_path,
                    stage="test",
                    exit_code=test_ret,
                )
            post_change_state.set_step(state, "test", "done", test_ret)
        else:
            post_change_state.set_step(state, "test", "skipped")

        print("--- post-change: completed.")
        state["next_action"] = "Done."
        return post_change_state.finish(
            state=state,
            state_path=state_path,
            exit_code=0,
            status="completed",
        )

    def _run_tests(self, app_name: str, build_dir_name: str, concise: bool) -> int:
        return VerifyCommand(self.ctx).run_tests(
            app_name=app_name,
            build_dir_name=build_dir_name,
            concise=concise,
        )
