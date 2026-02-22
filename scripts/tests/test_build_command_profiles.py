import sys
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPTS_DIR = REPO_ROOT / "scripts"

if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))

from toolchain.commands.cmd_build import BuildCommand  # noqa: E402
from toolchain.core.context import Context  # noqa: E402


class TestBuildCommandProfiles(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ctx = Context(REPO_ROOT)
        cls.command = BuildCommand(cls.ctx)

    def test_resolve_build_dir_by_profile(self):
        self.assertEqual(
            self.command.resolve_build_dir_name(tidy=False, profile_name="fast"),
            "build_fast",
        )
        self.assertEqual(
            self.command.resolve_build_dir_name(tidy=False, profile_name="release_safe"),
            "build",
        )
        self.assertEqual(
            self.command.resolve_build_dir_name(tidy=False, profile_name="check"),
            "build_check",
        )

    def test_resolve_build_dir_gradle_backend_defaults_to_build(self):
        self.assertEqual(
            self.command.resolve_build_dir_name(
                tidy=False,
                profile_name="fast",
                app_name="tracer_android",
            ),
            "build",
        )
        self.assertEqual(
            self.command.resolve_build_dir_name(
                tidy=True,
                profile_name="check",
                app_name="tracer_android",
            ),
            "build",
        )
        self.assertEqual(
            self.command.resolve_build_dir_name(
                tidy=False,
                build_dir_name="custom_build_dir",
                profile_name="fast",
                app_name="tracer_android",
            ),
            "build",
        )

    def test_gradle_backend_uses_profile_tasks(self):
        captured = {}

        def fake_run_command(cmd, cwd=None, env=None, log_file=None, flush_interval=20):
            captured["cmd"] = cmd
            captured["cwd"] = cwd
            return 0

        with patch(
            "toolchain.commands.cmd_build.command.run_command", side_effect=fake_run_command
        ):
            result = self.command.build(
                app_name="tracer_android",
                tidy=False,
                extra_args=[],
                cmake_args=["-D", "IGNORED=ON"],
                profile_name="release_safe",
            )

        self.assertEqual(result, 0)
        self.assertIn(":app:assembleRelease", captured["cmd"])
        self.assertIn("-PtimeTracerDisableNativeOptimization=true", captured["cmd"])
        self.assertTrue(any(arg.startswith("-PtimeTracerConfigRoot=") for arg in captured["cmd"]))
        self.assertEqual(captured["cwd"], self.ctx.get_app_dir("tracer_android"))

    def test_gradle_backend_auto_syncs_android_config_before_build(self):
        calls: list[dict] = []

        def fake_run_command(cmd, cwd=None, env=None, log_file=None, flush_interval=20):
            calls.append({"cmd": cmd, "cwd": cwd})
            return 0

        with patch(
            "toolchain.commands.cmd_build.command.run_command", side_effect=fake_run_command
        ):
            result = self.command.build(
                app_name="tracer_android",
                tidy=False,
                extra_args=[],
                cmake_args=[],
                profile_name="fast",
            )

        self.assertEqual(result, 0)
        self.assertGreaterEqual(len(calls), 2)
        sync_cmd = " ".join(calls[0]["cmd"])
        self.assertIn("platform_config", sync_cmd)
        self.assertIn("run.py", sync_cmd)
        self.assertIn("--target", calls[0]["cmd"])
        self.assertIn("android", calls[0]["cmd"])
        self.assertIn("--source-root", calls[0]["cmd"])
        self.assertIn("--android-output-root", calls[0]["cmd"])
        self.assertIn("-PtimeTracerDisableNativeOptimization=true", calls[-1]["cmd"])
        self.assertTrue(any(arg.startswith("-PtimeTracerConfigRoot=") for arg in calls[-1]["cmd"]))
        self.assertIn(":app:assembleDebug", calls[-1]["cmd"])

    def test_gradle_backend_keeps_explicit_native_optimization_override(self):
        captured = {}

        def fake_run_command(cmd, cwd=None, env=None, log_file=None, flush_interval=20):
            captured["cmd"] = cmd
            captured["cwd"] = cwd
            return 0

        with patch(
            "toolchain.commands.cmd_build.command.run_command", side_effect=fake_run_command
        ):
            result = self.command.build(
                app_name="tracer_android",
                tidy=False,
                extra_args=["-PtimeTracerDisableNativeOptimization=false"],
                cmake_args=[],
                profile_name="fast",
            )

        self.assertEqual(result, 0)
        self.assertIn("-PtimeTracerDisableNativeOptimization=false", captured["cmd"])
        self.assertNotIn("-PtimeTracerDisableNativeOptimization=true", captured["cmd"])
        self.assertIn(":app:assembleDebug", captured["cmd"])

    def test_gradle_backend_rejects_non_canonical_source_config_override(self):
        calls: list[dict] = []

        def fake_run_command(cmd, cwd=None, env=None, log_file=None, flush_interval=20):
            calls.append({"cmd": cmd, "cwd": cwd})
            return 0

        invalid_source = self.ctx.repo_root / "apps" / "tracer_windows_cli" / "config"
        with patch(
            "toolchain.commands.cmd_build.command.run_command", side_effect=fake_run_command
        ):
            result = self.command.build(
                app_name="tracer_android",
                tidy=False,
                extra_args=[f"-PtimeTracerSourceConfigRoot={invalid_source}"],
                cmake_args=[],
                profile_name="fast",
            )

        self.assertEqual(result, 2)
        # Only sync command should run before validation fails.
        self.assertEqual(len(calls), 1)
        self.assertIn("platform_config", " ".join(calls[0]["cmd"]))

    def test_gradle_backend_rejects_non_generated_android_config_root(self):
        calls: list[dict] = []

        def fake_run_command(cmd, cwd=None, env=None, log_file=None, flush_interval=20):
            calls.append({"cmd": cmd, "cwd": cwd})
            return 0

        invalid_output = self.ctx.repo_root / "apps" / "tracer_android" / "build" / "tmp"
        with patch(
            "toolchain.commands.cmd_build.command.run_command", side_effect=fake_run_command
        ):
            result = self.command.build(
                app_name="tracer_android",
                tidy=False,
                extra_args=[f"-PtimeTracerConfigRoot={invalid_output}"],
                cmake_args=[],
                profile_name="fast",
            )

        self.assertEqual(result, 2)
        # Only sync command should run before validation fails.
        self.assertEqual(len(calls), 1)
        self.assertIn("platform_config", " ".join(calls[0]["cmd"]))

    def test_cmake_backend_injects_windows_config_source_dir(self):
        calls: list[dict] = []

        def fake_run_command(cmd, cwd=None, env=None, log_file=None, flush_interval=20):
            calls.append({"cmd": cmd, "cwd": cwd})
            return 0

        with (
            patch("toolchain.commands.cmd_build.command.run_command", side_effect=fake_run_command),
            patch.object(BuildCommand, "_is_configured", return_value=False),
        ):
            result = self.command.build(
                app_name="tracer_windows_cli",
                tidy=False,
                extra_args=[],
                cmake_args=[],
                profile_name="fast",
            )

        self.assertEqual(result, 0)
        self.assertGreaterEqual(len(calls), 3)
        self.assertIn("--target", calls[0]["cmd"])
        self.assertIn("windows", calls[0]["cmd"])
        self.assertIn("--windows-output-root", calls[0]["cmd"])

        configure_call = next(
            call
            for call in calls
            if call["cmd"] and call["cmd"][0] == "cmake" and "-S" in call["cmd"]
        )
        self.assertTrue(
            any(
                arg.startswith("TRACER_WINDOWS_CONFIG_SOURCE_DIR=") for arg in configure_call["cmd"]
            )
        )

    def test_cmake_backend_syncs_runtime_config_after_build(self):
        calls: list[dict] = []

        def fake_run_command(cmd, cwd=None, env=None, log_file=None, flush_interval=20):
            calls.append({"cmd": cmd, "cwd": cwd})
            return 0

        with (
            patch("toolchain.commands.cmd_build.command.run_command", side_effect=fake_run_command),
            patch.object(BuildCommand, "_is_configured", return_value=True),
            patch.object(
                BuildCommand,
                "_needs_windows_config_reconfigure",
                return_value=False,
            ),
            patch.object(
                BuildCommand,
                "_sync_windows_runtime_config_copy_if_needed",
                return_value=0,
            ) as runtime_sync,
        ):
            result = self.command.build(
                app_name="tracer_windows_cli",
                tidy=False,
                extra_args=[],
                cmake_args=[],
                profile_name="fast",
            )

        self.assertEqual(result, 0)
        self.assertGreaterEqual(len(calls), 2)
        runtime_sync.assert_called_once_with(
            app_name="tracer_windows_cli",
            build_dir_name="build_fast",
        )

    def test_cmake_backend_rejects_legacy_source_config_override(self):
        calls: list[dict] = []

        def fake_run_command(cmd, cwd=None, env=None, log_file=None, flush_interval=20):
            calls.append({"cmd": cmd, "cwd": cwd})
            return 0

        legacy_source = self.ctx.repo_root / "apps" / "time_tracer" / "config"
        with (
            patch("toolchain.commands.cmd_build.command.run_command", side_effect=fake_run_command),
            patch.object(BuildCommand, "_is_configured", return_value=False),
        ):
            result = self.command.build(
                app_name="tracer_windows_cli",
                tidy=False,
                extra_args=[],
                cmake_args=[
                    "-D",
                    f"TRACER_WINDOWS_CONFIG_SOURCE_DIR={legacy_source}",
                ],
                profile_name="fast",
            )

        self.assertEqual(result, 2)
        configure_calls = [
            call
            for call in calls
            if call["cmd"] and call["cmd"][0] == "cmake" and "-S" in call["cmd"]
        ]
        self.assertEqual(configure_calls, [])
