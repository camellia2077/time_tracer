import io
import sys
from contextlib import redirect_stderr, redirect_stdout
from unittest.mock import patch

from ._run_cli_dispatch_test_support import RunCliDispatchTestBase


class TestRunCliDispatchCoreFormat(RunCliDispatchTestBase):
    def test_format_dispatches_path_mode_and_check_flag(self):
        class FakeFormatCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeFormatCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.quality.format.FormatCommand", FakeFormatCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "format",
                    "--paths",
                    "libs",
                    "apps/tracer_core_shell",
                    "--check",
                ]
            )

        self.assertEqual(FakeFormatCommand.last_kwargs["app_name"], None)
        self.assertEqual(
            FakeFormatCommand.last_kwargs["raw_paths"],
            ["libs", "apps/tracer_core_shell"],
        )
        self.assertTrue(FakeFormatCommand.last_kwargs["check_only"])

    def test_format_requires_app_or_paths(self):
        stdout = io.StringIO()
        stderr = io.StringIO()
        with patch.object(
            sys,
            "argv",
            ["run.py", "format"],
        ), redirect_stdout(stdout), redirect_stderr(stderr):
            rc = self.run_module.main()

        self.assertEqual(rc, 2)
        self.assertIn("requires either `--app` or `--paths`", stderr.getvalue())
        self.assertEqual(stdout.getvalue(), "")

    def test_unrecognized_arguments_show_build_verify_hint(self):
        stderr = io.StringIO()
        with patch.object(
            sys,
            "argv",
            ["run.py", "build", "--app", "tracer_core_shell", "--bad-arg"],
        ), redirect_stderr(stderr), self.assertRaises(SystemExit) as raised:
            self.run_module.main()

        self.assertEqual(raised.exception.code, 2)
        error_text = stderr.getvalue()
        self.assertIn("unrecognized arguments: --bad-arg", error_text)
        self.assertIn("python tools/run.py build -h", error_text)
        self.assertIn("python tools/run.py build ...", error_text)
        self.assertIn("python tools/run.py verify ...", error_text)
        self.assertNotIn("python tools/run.py validate", error_text)
