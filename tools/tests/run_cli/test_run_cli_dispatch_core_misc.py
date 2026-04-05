import argparse
import io
from contextlib import redirect_stderr, redirect_stdout
from unittest.mock import patch

from ._run_cli_dispatch_test_support import Context, REPO_ROOT, RunCliDispatchTestBase, dispatch_command


class TestRunCliDispatchCoreMisc(RunCliDispatchTestBase):
    def test_dispatch_unsupported_command_writes_error_to_stderr(self):
        stdout = io.StringIO()
        stderr = io.StringIO()
        ctx = Context(REPO_ROOT)
        args = argparse.Namespace(command="unsupported", app=None, app_path=None)

        with redirect_stdout(stdout), redirect_stderr(stderr):
            rc = dispatch_command(args, ctx)

        self.assertEqual(rc, 2)
        self.assertIn("unsupported command `unsupported`", stderr.getvalue())
        self.assertEqual(stdout.getvalue(), "")

    def test_dispatch_missing_app_for_app_path_writes_error_to_stderr(self):
        stdout = io.StringIO()
        stderr = io.StringIO()
        ctx = Context(REPO_ROOT)
        args = argparse.Namespace(command="build", app=None, app_path="apps/demo")

        with redirect_stdout(stdout), redirect_stderr(stderr):
            rc = dispatch_command(args, ctx)

        self.assertEqual(rc, 2)
        self.assertIn("--app-path requires --app", stderr.getvalue())
        self.assertEqual(stdout.getvalue(), "")

    def test_report_markdown_gate_dispatches_build_dir_and_refresh(self):
        class FakeReportMarkdownGateCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeReportMarkdownGateCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.quality.report_markdown_gate.ReportMarkdownGateCommand",
            FakeReportMarkdownGateCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "report-markdown-gate",
                    "--app",
                    "tracer_core_shell",
                    "--build-dir",
                    "build_release",
                    "--refresh-golden",
                ]
            )

        self.assertEqual(
            FakeReportMarkdownGateCommand.last_kwargs["app_name"],
            "tracer_core_shell",
        )
        self.assertEqual(
            FakeReportMarkdownGateCommand.last_kwargs["build_dir_name"],
            "build_release",
        )
        self.assertTrue(FakeReportMarkdownGateCommand.last_kwargs["refresh_golden"])

    def test_config_migrate_defaults_to_dry_run(self):
        class FakeConfigMigrateCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeConfigMigrateCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.config_migrate.ConfigMigrateCommand",
            FakeConfigMigrateCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "config-migrate",
                    "--app",
                    "tracer_windows_rust_cli",
                ]
            )

        self.assertIsNotNone(FakeConfigMigrateCommand.last_kwargs)
        self.assertEqual(
            FakeConfigMigrateCommand.last_kwargs["app_name"],
            "tracer_windows_rust_cli",
        )
        self.assertTrue(FakeConfigMigrateCommand.last_kwargs["dry_run"])
        self.assertFalse(FakeConfigMigrateCommand.last_kwargs["apply_changes"])
        self.assertFalse(FakeConfigMigrateCommand.last_kwargs["rollback"])

    def test_self_test_dispatches_group(self):
        class FakeSelfTestCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeSelfTestCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.quality.self_test.SelfTestCommand",
            FakeSelfTestCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "self-test",
                    "--group",
                    "verify-stack",
                    "--quiet",
                ]
            )

        self.assertEqual(FakeSelfTestCommand.last_kwargs["group"], "verify-stack")
        self.assertFalse(FakeSelfTestCommand.last_kwargs["verbose"])
