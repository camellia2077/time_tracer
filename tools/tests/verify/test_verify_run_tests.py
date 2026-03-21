from unittest.mock import patch

from .test_verify_fixtures import VerifyCommandTestBase


class TestVerifyRunTests(VerifyCommandTestBase):
    def test_run_tests_skips_when_suite_unmapped(self):
        with patch("tools.toolchain.commands.cmd_quality.verify.run_command") as mocked_run:
            result = self.run_tests_silently(
                app_name="unknown_app",
                build_dir_name="build_fast",
                concise=False,
            )
        self.assertEqual(result, 0)
        mocked_run.assert_not_called()

    def test_run_tests_tracer_android_uses_bin_dir(self):
        with (
            patch(
                "tools.toolchain.commands.cmd_quality.verify.run_command", return_value=0
            ) as mocked_run,
            patch(
                "tools.toolchain.commands.cmd_quality.verify.resolve_suite_bin_dir",
                return_value=r"C:\Windows\System32",
            ),
        ):
            result = self.run_tests_silently(
                app_name="tracer_android",
                build_dir_name="build_fast",
                concise=False,
            )

        self.assertEqual(result, 0)
        called_cmd = mocked_run.call_args.args[0]
        self.assertIn("--suite", called_cmd)
        self.assertIn("artifact_android", called_cmd)
        self.assertIn("--bin-dir", called_cmd)
        self.assertIn(r"C:\Windows\System32", called_cmd)
        self.assertNotIn("--build-dir", called_cmd)

    def test_run_tests_tracer_android_android_style_uses_profile_config(self):
        with (
            patch(
                "tools.toolchain.commands.cmd_quality.verify.run_command", return_value=0
            ) as mocked_run,
            patch(
                "tools.toolchain.commands.cmd_quality.verify.resolve_suite_bin_dir",
                return_value=r"C:\Windows\System32",
            ),
        ):
            result = self.run_tests_silently(
                app_name="tracer_android",
                build_dir_name="build_fast",
                profile_name="android_style",
                concise=True,
            )

        self.assertEqual(result, 0)
        called_cmd = mocked_run.call_args.args[0]
        self.assertIn("--config", called_cmd)
        self.assertIn("config_android_style.toml", called_cmd)

    def test_run_tests_tracer_core_uses_artifact_windows_cli_suite(self):
        with patch(
            "tools.toolchain.commands.cmd_quality.verify.run_command", return_value=0
        ) as mocked_run:
            result = self.run_tests_silently(
                app_name="tracer_core",
                build_dir_name="build_fast",
                concise=True,
            )

        self.assertEqual(result, 0)
        called_cmd = None
        for call in mocked_run.call_args_list:
            cmd = call.args[0]
            if "--suite" in cmd and "artifact_windows_cli" in cmd:
                called_cmd = cmd
                break
        self.assertIsNotNone(called_cmd)
        self.assertIn("--suite", called_cmd)
        self.assertIn("artifact_windows_cli", called_cmd)
        self.assertIn("--with-build", called_cmd)
        self.assertIn("--skip-configure", called_cmd)
        self.assertIn("--concise", called_cmd)
