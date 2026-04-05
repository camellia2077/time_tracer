from unittest.mock import patch

from ._run_cli_dispatch_test_support import RunCliDispatchTestBase


class TestRunCliDispatchTidyStep(RunCliDispatchTestBase):
    def test_tidy_step_dispatches_verify_related_flags(self):
        class FakeTidyStepCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyStepCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_step.TidyStepCommand",
            FakeTidyStepCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-step",
                    "--task-log",
                    "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.json",
                    "--build-dir",
                    "build_fast",
                    "--profile",
                    "fast",
                    "--concise",
                    "--kill-build-procs",
                ]
            )

        self.assertEqual(
            FakeTidyStepCommand.last_kwargs["task_log_path"],
            "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.json",
        )
        self.assertEqual(FakeTidyStepCommand.last_kwargs["verify_build_dir_name"], "build_fast")
        self.assertEqual(FakeTidyStepCommand.last_kwargs["profile_name"], "fast")
        self.assertTrue(FakeTidyStepCommand.last_kwargs["concise"])
        self.assertTrue(FakeTidyStepCommand.last_kwargs["kill_build_procs"])

    def test_tidy_step_dispatches_config_file(self):
        class FakeTidyStepCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyStepCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_step.TidyStepCommand",
            FakeTidyStepCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-step",
                    "--task-log",
                    "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.json",
                    "--config-file",
                    ".clang-tidy.strict",
                ]
            )

        self.assertEqual(FakeTidyStepCommand.last_kwargs["config_file"], ".clang-tidy.strict")
