from unittest.mock import patch

from ._run_cli_dispatch_test_support import RunCliDispatchTestBase


class TestRunCliDispatchTidySourceStep(RunCliDispatchTestBase):
    def test_tidy_source_step_dispatches_verify_related_flags(self):
        class FakeTidySourceStepCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidySourceStepCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_source_step.TidySourceStepCommand",
            FakeTidySourceStepCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-source-step",
                    "--task-log",
                    "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/clusters/cluster_001/task_001.json",
                    "--build-dir",
                    "build_fast",
                    "--profile",
                    "fast",
                    "--concise",
                    "--kill-build-procs",
                    "--strict",
                ]
            )

        self.assertEqual(
            FakeTidySourceStepCommand.last_kwargs["task_log_path"],
            "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/clusters/cluster_001/task_001.json",
        )
        self.assertEqual(FakeTidySourceStepCommand.last_kwargs["verify_build_dir_name"], "build_fast")
        self.assertEqual(FakeTidySourceStepCommand.last_kwargs["profile_name"], "fast")
        self.assertTrue(FakeTidySourceStepCommand.last_kwargs["concise"])
        self.assertTrue(FakeTidySourceStepCommand.last_kwargs["kill_build_procs"])
        self.assertTrue(FakeTidySourceStepCommand.last_kwargs["strict"])
