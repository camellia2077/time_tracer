from unittest.mock import patch

from ._run_cli_dispatch_test_support import RunCliDispatchTestBase


class TestRunCliDispatchCoreAnalyze(RunCliDispatchTestBase):
    def test_analyze_dispatches_source_scope_build_dir_and_profile(self):
        class FakeAnalyzeCommand:
            last_args = None
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeAnalyzeCommand.last_args = args
                FakeAnalyzeCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.analyze.AnalyzeCommand", FakeAnalyzeCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "analyze",
                    "--app",
                    "tracer_core_shell",
                    "--source-scope",
                    "core_family",
                    "--build-dir",
                    "build_analyze_core_family",
                    "--profile",
                    "fast",
                    "--jobs",
                    "8",
                ]
            )

        self.assertEqual(FakeAnalyzeCommand.last_args, ("tracer_core_shell",))
        self.assertEqual(FakeAnalyzeCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(
            FakeAnalyzeCommand.last_kwargs["build_dir_name"],
            "build_analyze_core_family",
        )
        self.assertEqual(FakeAnalyzeCommand.last_kwargs["profile_name"], "fast")
        self.assertEqual(FakeAnalyzeCommand.last_kwargs["jobs"], 8)

    def test_analyze_split_dispatches_scope_build_dir_and_batch_size(self):
        class FakeAnalyzeCommand:
            last_args = None
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def split_only(self, *args, **kwargs):
                FakeAnalyzeCommand.last_args = args
                FakeAnalyzeCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.analyze_split.AnalyzeCommand",
            FakeAnalyzeCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "analyze-split",
                    "--app",
                    "tracer_core_shell",
                    "--source-scope",
                    "core_family",
                    "--build-dir",
                    "build_analyze_core_family",
                    "--batch-size",
                    "7",
                ]
            )

        self.assertEqual(FakeAnalyzeCommand.last_args, ("tracer_core_shell",))
        self.assertEqual(FakeAnalyzeCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(
            FakeAnalyzeCommand.last_kwargs["build_dir_name"],
            "build_analyze_core_family",
        )
        self.assertEqual(FakeAnalyzeCommand.last_kwargs["batch_size"], 7)
