import sys
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPTS_DIR = REPO_ROOT / "scripts"

if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))

from toolchain.commands.cmd_quality.verify import VerifyCommand  # noqa: E402
from toolchain.core.context import Context  # noqa: E402


class TestVerifyCommand(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ctx = Context(REPO_ROOT)
        cls.command = VerifyCommand(cls.ctx)

    def test_run_tests_skips_when_suite_unmapped(self):
        with patch("toolchain.commands.cmd_quality.verify.run_command") as mocked_run:
            result = self.command.run_tests(
                app_name="unknown_app",
                build_dir_name="build_fast",
                concise=False,
            )
        self.assertEqual(result, 0)
        mocked_run.assert_not_called()

    def test_run_tests_tracer_android_uses_bin_dir(self):
        with (
            patch(
                "toolchain.commands.cmd_quality.verify.run_command", return_value=0
            ) as mocked_run,
            patch(
                "toolchain.commands.cmd_quality.verify.resolve_suite_bin_dir",
                return_value=r"C:\Windows\System32",
            ),
        ):
            result = self.command.run_tests(
                app_name="tracer_android",
                build_dir_name="build_fast",
                concise=False,
            )

        self.assertEqual(result, 0)
        called_cmd = mocked_run.call_args.args[0]
        self.assertIn("--suite", called_cmd)
        self.assertIn("tracer_android", called_cmd)
        self.assertIn("--bin-dir", called_cmd)
        self.assertIn(r"C:\Windows\System32", called_cmd)
        self.assertNotIn("--build-dir", called_cmd)

    def test_run_tests_tracer_android_android_style_uses_profile_config(self):
        with (
            patch(
                "toolchain.commands.cmd_quality.verify.run_command", return_value=0
            ) as mocked_run,
            patch(
                "toolchain.commands.cmd_quality.verify.resolve_suite_bin_dir",
                return_value=r"C:\Windows\System32",
            ),
        ):
            result = self.command.run_tests(
                app_name="tracer_android",
                build_dir_name="build_fast",
                profile_name="android_style",
                concise=True,
            )

        self.assertEqual(result, 0)
        called_cmd = mocked_run.call_args.args[0]
        self.assertIn("--config", called_cmd)
        self.assertIn("config_android_style.toml", called_cmd)

    def test_run_tests_time_tracer_uses_tracer_windows_cli_suite(self):
        with patch(
            "toolchain.commands.cmd_quality.verify.run_command", return_value=0
        ) as mocked_run:
            result = self.command.run_tests(
                app_name="time_tracer",
                build_dir_name="build_fast",
                concise=True,
            )

        self.assertEqual(result, 0)
        called_cmd = None
        for call in mocked_run.call_args_list:
            cmd = call.args[0]
            if "--suite" in cmd and "tracer_windows_cli" in cmd:
                called_cmd = cmd
                break
        self.assertIsNotNone(called_cmd)
        self.assertIn("--suite", called_cmd)
        self.assertIn("tracer_windows_cli", called_cmd)
        self.assertIn("--with-build", called_cmd)
        self.assertIn("--skip-configure", called_cmd)
        self.assertIn("--concise", called_cmd)

    def test_execute_time_tracer_builds_tracer_windows_cli_then_runs_suite(self):
        class FakeBuildCommand:
            last_build_kwargs = None
            last_resolve_kwargs = None

            def __init__(self, _ctx):
                pass

            def build(self, **kwargs):
                FakeBuildCommand.last_build_kwargs = kwargs
                return 0

            def resolve_build_dir_name(self, **kwargs):
                FakeBuildCommand.last_resolve_kwargs = kwargs
                return "build_fast"

        with patch("toolchain.commands.cmd_quality.verify.BuildCommand", FakeBuildCommand):
            with patch(
                "toolchain.commands.cmd_quality.verify.run_command", return_value=0
            ) as mocked_run:
                result = self.command.execute(
                    app_name="time_tracer",
                    build_dir_name="build_fast",
                    concise=True,
                )

        self.assertEqual(result, 0)
        self.assertIsNotNone(FakeBuildCommand.last_build_kwargs)
        self.assertEqual(
            FakeBuildCommand.last_build_kwargs["app_name"],
            "tracer_windows_cli",
        )
        called_cmd = None
        for call in mocked_run.call_args_list:
            cmd = call.args[0]
            if "--suite" in cmd and "tracer_windows_cli" in cmd:
                called_cmd = cmd
                break
        self.assertIsNotNone(called_cmd)
        self.assertIn("--suite", called_cmd)
        self.assertIn("tracer_windows_cli", called_cmd)
        self.assertNotIn("--with-build", called_cmd)
        self.assertNotIn("--skip-configure", called_cmd)
        self.assertIn("--concise", called_cmd)

    def test_execute_unmapped_app_writes_build_only_result_on_success(self):
        class FakeBuildCommand:
            def __init__(self, _ctx):
                pass

            def build(self, **_kwargs):
                return 0

            def resolve_build_dir_name(self, **_kwargs):
                return "build_fast"

        with (
            patch("toolchain.commands.cmd_quality.verify.BuildCommand", FakeBuildCommand),
            patch.object(
                VerifyCommand,
                "_write_build_only_result_json",
                return_value=None,
            ) as mocked_writer,
        ):
            result = self.command.execute(
                app_name="unknown_app",
                build_dir_name="build_fast",
                concise=True,
            )

        self.assertEqual(result, 0)
        mocked_writer.assert_called_once()
        call_kwargs = mocked_writer.call_args.kwargs
        self.assertEqual(call_kwargs["app_name"], "unknown_app")
        self.assertEqual(call_kwargs["build_dir_name"], "build_fast")
        self.assertTrue(call_kwargs["success"])
        self.assertEqual(call_kwargs["exit_code"], 0)

    def test_execute_unmapped_app_writes_build_only_result_on_build_failure(self):
        class FakeBuildCommand:
            def __init__(self, _ctx):
                pass

            def build(self, **_kwargs):
                return 3

            def resolve_build_dir_name(self, **_kwargs):
                return "build_fast"

        with (
            patch("toolchain.commands.cmd_quality.verify.BuildCommand", FakeBuildCommand),
            patch.object(
                VerifyCommand,
                "_write_build_only_result_json",
                return_value=None,
            ) as mocked_writer,
        ):
            result = self.command.execute(
                app_name="unknown_app",
                build_dir_name="build_fast",
                concise=True,
            )

        self.assertEqual(result, 3)
        mocked_writer.assert_called_once()
        call_kwargs = mocked_writer.call_args.kwargs
        self.assertEqual(call_kwargs["app_name"], "unknown_app")
        self.assertEqual(call_kwargs["build_dir_name"], "build_fast")
        self.assertFalse(call_kwargs["success"])
        self.assertEqual(call_kwargs["exit_code"], 3)

    def test_execute_mapped_app_writes_result_on_build_failure(self):
        class FakeBuildCommand:
            def __init__(self, _ctx):
                pass

            def build(self, **_kwargs):
                return 5

            def resolve_build_dir_name(self, **_kwargs):
                return "build_fast"

        with (
            patch("toolchain.commands.cmd_quality.verify.BuildCommand", FakeBuildCommand),
            patch.object(
                VerifyCommand,
                "_write_build_only_result_json",
                return_value=None,
            ) as mocked_writer,
        ):
            result = self.command.execute(
                app_name="tracer_android",
                build_dir_name="build_fast",
                profile_name="android_style",
                concise=True,
            )

        self.assertEqual(result, 5)
        mocked_writer.assert_called_once()
        call_kwargs = mocked_writer.call_args.kwargs
        self.assertEqual(call_kwargs["app_name"], "tracer_android")
        self.assertEqual(call_kwargs["build_dir_name"], "build_fast")
        self.assertFalse(call_kwargs["success"])
        self.assertEqual(call_kwargs["exit_code"], 5)
        self.assertFalse(call_kwargs["build_only"])
