import io
import sys
from contextlib import redirect_stderr
from unittest.mock import patch

from ._run_cli_dispatch_test_support import RunCliDispatchTestBase


class TestRunCliDispatchTidyTask(RunCliDispatchTestBase):
    def test_tidy_task_fix_dispatches_task_log_and_dry_run(self):
        class FakeTidyTaskFixCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyTaskFixCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_task_fix.TidyTaskFixCommand",
            FakeTidyTaskFixCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-task-fix",
                    "--task-log",
                    "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.json",
                    "--dry-run",
                ]
            )

        self.assertEqual(
            FakeTidyTaskFixCommand.last_kwargs["task_log_path"],
            "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.json",
        )
        self.assertTrue(FakeTidyTaskFixCommand.last_kwargs["dry_run"])

    def test_tidy_task_patch_dispatches_task_log(self):
        class FakeTidyTaskPatchCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyTaskPatchCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_task_patch.TidyTaskPatchCommand",
            FakeTidyTaskPatchCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-task-patch",
                    "--task-log",
                    "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.log",
                ]
            )

        self.assertEqual(
            FakeTidyTaskPatchCommand.last_kwargs["task_log_path"],
            "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.log",
        )

    def test_tidy_task_suggest_dispatches_task_log(self):
        class FakeTidyTaskSuggestCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyTaskSuggestCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_task_suggest.TidyTaskSuggestCommand",
            FakeTidyTaskSuggestCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-task-suggest",
                    "--task-log",
                    "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.json",
                ]
            )

        self.assertEqual(
            FakeTidyTaskSuggestCommand.last_kwargs["task_log_path"],
            "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.json",
        )

    def test_tidy_task_patch_requires_task_log(self):
        stderr = io.StringIO()
        with patch.object(
            sys,
            "argv",
            ["run.py", "tidy-task-patch"],
        ), redirect_stderr(stderr), self.assertRaises(SystemExit) as raised:
            self.run_module.main()

        self.assertEqual(raised.exception.code, 2)
        self.assertIn("--task-log", stderr.getvalue())
