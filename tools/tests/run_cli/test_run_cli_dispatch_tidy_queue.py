import io
import sys
from contextlib import redirect_stderr
from unittest.mock import patch

from ._run_cli_dispatch_test_support import RunCliDispatchTestBase


class TestRunCliDispatchTidyQueue(RunCliDispatchTestBase):
    def test_tidy_dispatches_source_scope_and_build_dir(self):
        class FakeTidyCommand:
            last_args = None
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_args = args
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy",
                    "--app",
                    "tracer_core_shell",
                    "--source-scope",
                    "core_family",
                    "--build-dir",
                    "build_tidy_core_family",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_args, ("tracer_core_shell", []))
        self.assertEqual(FakeTidyCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(FakeTidyCommand.last_kwargs["build_dir_name"], "build_tidy_core_family")
        self.assertIsNone(FakeTidyCommand.last_kwargs["task_view"])

    def test_tidy_dispatches_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy",
                    "--app",
                    "tracer_core_shell",
                    "--task-view",
                    "text+toon",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "text+toon")

    def test_tidy_dispatches_strict_config(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy",
                    "--app",
                    "tracer_core_shell",
                    "--strict-config",
                ]
            )

        self.assertTrue(FakeTidyCommand.last_kwargs["strict_config"])

    def test_tidy_refresh_dispatches_config_file(self):
        class FakeTidyRefreshCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyRefreshCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_refresh.TidyRefreshCommand",
            FakeTidyRefreshCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-refresh",
                    "--app",
                    "tracer_core_shell",
                    "--batch-id",
                    "003",
                    "--config-file",
                    ".clang-tidy.strict",
                ]
            )

        self.assertEqual(FakeTidyRefreshCommand.last_kwargs["config_file"], ".clang-tidy.strict")

    def test_tidy_batch_dispatches_strict_config(self):
        class FakeTidyBatchCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyBatchCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_batch.TidyBatchCommand",
            FakeTidyBatchCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-batch",
                    "--app",
                    "tracer_core_shell",
                    "--batch-id",
                    "001",
                    "--strict-config",
                ]
            )

        self.assertTrue(FakeTidyBatchCommand.last_kwargs["strict_config"])

    def test_tidy_close_dispatches_stabilize(self):
        class FakeTidyCloseCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyCloseCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_close.TidyCloseCommand",
            FakeTidyCloseCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-close",
                    "--app",
                    "tracer_core_shell",
                    "--stabilize",
                    "--jobs",
                    "4",
                    "--parse-workers",
                    "6",
                ]
            )

        self.assertTrue(FakeTidyCloseCommand.last_kwargs["stabilize"])
        self.assertEqual(FakeTidyCloseCommand.last_kwargs["jobs"], 4)
        self.assertEqual(FakeTidyCloseCommand.last_kwargs["parse_workers"], 6)

    def test_tidy_close_strict_config_enables_stabilize_by_default(self):
        class FakeTidyCloseCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyCloseCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_close.TidyCloseCommand",
            FakeTidyCloseCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-close",
                    "--app",
                    "tracer_core_shell",
                    "--strict-config",
                ]
            )

        self.assertTrue(FakeTidyCloseCommand.last_kwargs["strict_config"])
        self.assertTrue(FakeTidyCloseCommand.last_kwargs["stabilize"])

    def test_tidy_dispatches_toon_only_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy",
                    "--app",
                    "tracer_core_shell",
                    "--task-view",
                    "toon",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "toon")

    def test_tidy_dispatches_json_only_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy",
                    "--app",
                    "tracer_core_shell",
                    "--task-view",
                    "json",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "json")

    def test_tidy_split_dispatches_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def split_only(self, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy_split.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-split",
                    "--app",
                    "tracer_core_shell",
                    "--task-view",
                    "text+toon",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "text+toon")

    def test_tidy_split_dispatches_text_only_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def split_only(self, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy_split.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-split",
                    "--app",
                    "tracer_core_shell",
                    "--task-view",
                    "text",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "text")

    def test_tidy_split_rejects_toon_only_task_view(self):
        stderr = io.StringIO()
        with patch.object(
            sys,
            "argv",
            ["run.py", "tidy-split", "--app", "tracer_core_shell", "--task-view", "toon"],
        ), redirect_stderr(stderr), self.assertRaises(SystemExit) as raised:
            self.run_module.main()

        self.assertEqual(raised.exception.code, 2)
        self.assertIn("invalid choice", stderr.getvalue())
        self.assertIn("'toon'", stderr.getvalue())

    def test_tidy_split_rejects_json_only_task_view(self):
        stderr = io.StringIO()
        with patch.object(
            sys,
            "argv",
            ["run.py", "tidy-split", "--app", "tracer_core_shell", "--task-view", "json"],
        ), redirect_stderr(stderr), self.assertRaises(SystemExit) as raised:
            self.run_module.main()

        self.assertEqual(raised.exception.code, 2)
        self.assertIn("invalid choice", stderr.getvalue())
        self.assertIn("'json'", stderr.getvalue())

    def test_tidy_fix_dispatches_source_scope_and_tidy_build_dir(self):
        class FakeTidyFixCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyFixCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_fix.TidyFixCommand",
            FakeTidyFixCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-fix",
                    "--app",
                    "tracer_core_shell",
                    "--source-scope",
                    "core_family",
                    "--tidy-build-dir",
                    "build_tidy_core_family",
                ]
            )

        self.assertEqual(FakeTidyFixCommand.last_kwargs["app_name"], "tracer_core_shell")
        self.assertEqual(FakeTidyFixCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(
            FakeTidyFixCommand.last_kwargs["tidy_build_dir_name"],
            "build_tidy_core_family",
        )

    def test_tidy_fix_dispatches_strict_config(self):
        class FakeTidyFixCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyFixCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_fix.TidyFixCommand",
            FakeTidyFixCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-fix",
                    "--app",
                    "tracer_core_shell",
                    "--strict-config",
                ]
            )

        self.assertTrue(FakeTidyFixCommand.last_kwargs["strict_config"])

    def test_clean_dispatches_tidy_build_dir(self):
        class FakeCleanCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeCleanCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.clean.CleanCommand", FakeCleanCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "clean",
                    "--app",
                    "tracer_core_shell",
                    "--tidy-build-dir",
                    "build_tidy_core_family",
                    "001",
                ]
            )

        self.assertEqual(FakeCleanCommand.last_kwargs["app_name"], "tracer_core_shell")
        self.assertEqual(
            FakeCleanCommand.last_kwargs["tidy_build_dir_name"],
            "build_tidy_core_family",
        )
        self.assertEqual(FakeCleanCommand.last_kwargs["task_ids"], ["001"])

    def test_tidy_batch_dispatches_scope_and_tidy_build_dir(self):
        class FakeTidyBatchCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyBatchCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_batch.TidyBatchCommand",
            FakeTidyBatchCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-batch",
                    "--app",
                    "tracer_core_shell",
                    "--batch-id",
                    "001",
                    "--preset",
                    "sop",
                    "--source-scope",
                    "core_family",
                    "--tidy-build-dir",
                    "build_tidy_core_family",
                ]
            )

        self.assertEqual(FakeTidyBatchCommand.last_kwargs["app_name"], "tracer_core_shell")
        self.assertEqual(FakeTidyBatchCommand.last_kwargs["batch_id"], "001")
        self.assertEqual(FakeTidyBatchCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(
            FakeTidyBatchCommand.last_kwargs["tidy_build_dir_name"],
            "build_tidy_core_family",
        )

    def test_tidy_flow_dispatches_scope_and_tidy_build_dir(self):
        class FakeTidyFlowCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyFlowCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_flow.TidyFlowCommand",
            FakeTidyFlowCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-flow",
                    "--app",
                    "tracer_core_shell",
                    "--all",
                    "--source-scope",
                    "core_family",
                    "--tidy-build-dir",
                    "build_tidy_core_family",
                ]
            )

        self.assertEqual(FakeTidyFlowCommand.last_kwargs["app_name"], "tracer_core_shell")
        self.assertTrue(FakeTidyFlowCommand.last_kwargs["process_all"])
        self.assertEqual(FakeTidyFlowCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(
            FakeTidyFlowCommand.last_kwargs["tidy_build_dir_name"],
            "build_tidy_core_family",
        )
        self.assertIsNone(FakeTidyFlowCommand.last_kwargs["task_view"])

    def test_tidy_refresh_dispatches_task_view(self):
        class FakeTidyRefreshCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyRefreshCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_refresh.TidyRefreshCommand",
            FakeTidyRefreshCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-refresh",
                    "--app",
                    "tracer_core_shell",
                    "--batch-id",
                    "003",
                    "--task-view",
                    "toon",
                ]
            )

        self.assertEqual(FakeTidyRefreshCommand.last_kwargs["batch_id"], "003")
        self.assertEqual(FakeTidyRefreshCommand.last_kwargs["task_view"], "toon")

    def test_tidy_flow_dispatches_task_view(self):
        class FakeTidyFlowCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyFlowCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_flow.TidyFlowCommand",
            FakeTidyFlowCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-flow",
                    "--app",
                    "tracer_core_shell",
                    "--all",
                    "--task-view",
                    "text+toon",
                ]
            )

        self.assertEqual(FakeTidyFlowCommand.last_kwargs["task_view"], "text+toon")

    def test_tidy_dispatches_profile_concise_and_kill_build_procs(self):
        class FakeTidyCommand:
            last_args = None
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_args = args
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy",
                    "--app",
                    "tracer_core_shell",
                    "--profile",
                    "fast",
                    "--build-dir",
                    "build_tidy_core_family",
                    "--concise",
                    "--kill-build-procs",
                    "--source-scope",
                    "core_family",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_args, ("tracer_core_shell", []))
        self.assertEqual(FakeTidyCommand.last_kwargs["profile_name"], "fast")
        self.assertEqual(FakeTidyCommand.last_kwargs["build_dir_name"], "build_tidy_core_family")
        self.assertTrue(FakeTidyCommand.last_kwargs["concise"])
        self.assertTrue(FakeTidyCommand.last_kwargs["kill_build_procs"])
