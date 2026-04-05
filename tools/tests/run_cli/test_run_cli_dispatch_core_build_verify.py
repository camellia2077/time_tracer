import io
import sys
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import patch

from ._run_cli_dispatch_test_support import RunCliDispatchTestBase


class TestRunCliDispatchCoreBuildVerify(RunCliDispatchTestBase):
    def test_build_dispatches_profile_and_cmake_args(self):
        class FakeBuildCommand:
            last_args = None
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def build(self, *args, **kwargs):
                FakeBuildCommand.last_args = args
                FakeBuildCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.build.BuildCommand", FakeBuildCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "build",
                    "--app",
                    "tracer_core",
                    "--profile",
                    "fast",
                    "--cmake-args=-DA=1",
                    "--cmake-args=-DB=2",
                ]
            )

        self.assertIsNotNone(FakeBuildCommand.last_kwargs)
        self.assertEqual(FakeBuildCommand.last_args, ())
        self.assertEqual(FakeBuildCommand.last_kwargs["app_name"], "tracer_core")
        self.assertFalse(FakeBuildCommand.last_kwargs["tidy"])
        self.assertEqual(FakeBuildCommand.last_kwargs["extra_args"], [])
        self.assertEqual(FakeBuildCommand.last_kwargs["profile_name"], "fast")
        self.assertEqual(FakeBuildCommand.last_kwargs["cmake_args"], ["-DA=1", "-DB=2"])

    def test_build_dispatches_repeated_profiles_for_gradle_app(self):
        class FakeBuildCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def build(self, *args, **kwargs):
                FakeBuildCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.build.BuildCommand", FakeBuildCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "build",
                    "--app",
                    "tracer_android",
                    "--profile",
                    "android_style",
                    "--profile",
                    "android_ci",
                ]
            )

        self.assertEqual(
            FakeBuildCommand.last_kwargs["profile_name"],
            ["android_style", "android_ci"],
        )

    def test_build_dispatches_concise_and_kill_build_procs(self):
        class FakeBuildCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def build(self, *args, **kwargs):
                FakeBuildCommand.last_kwargs = kwargs
                return 0

            def resolve_output_log_path(self, **_kwargs):
                return Path.cwd() / "build.log"

        with patch("tools.toolchain.cli.handlers.build.BuildCommand", FakeBuildCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "build",
                    "--app",
                    "tracer_core",
                    "--profile",
                    "fast",
                    "--concise",
                    "--kill-build-procs",
                ]
            )

        self.assertTrue(FakeBuildCommand.last_kwargs["concise"])
        self.assertTrue(FakeBuildCommand.last_kwargs["kill_build_procs"])

    def test_build_dispatches_rust_cli_windows_runtime(self):
        class FakeBuildCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def build(self, *args, **kwargs):
                FakeBuildCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.build.BuildCommand", FakeBuildCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "build",
                    "--app",
                    "tracer_windows_rust_cli",
                    "--profile",
                    "release_bundle",
                    "--runtime-platform",
                    "windows",
                    "--rust-runtime-sync",
                    "strict",
                ]
            )

        self.assertEqual(FakeBuildCommand.last_kwargs["runtime_platform"], "windows")
        self.assertEqual(FakeBuildCommand.last_kwargs["rust_runtime_sync"], "strict")

    def test_build_defaults_rust_cli_runtime_sync_to_strict(self):
        class FakeBuildCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def build(self, *args, **kwargs):
                FakeBuildCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.build.BuildCommand", FakeBuildCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "build",
                    "--app",
                    "tracer_windows_rust_cli",
                    "--profile",
                    "release_bundle",
                    "--runtime-platform",
                    "windows",
                ]
            )

        self.assertEqual(FakeBuildCommand.last_kwargs["runtime_platform"], "windows")
        self.assertEqual(FakeBuildCommand.last_kwargs["rust_runtime_sync"], "strict")

    def test_build_dispatches_runtime_platform(self):
        class FakeBuildCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def build(self, *args, **kwargs):
                FakeBuildCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.build.BuildCommand", FakeBuildCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "build",
                    "--app",
                    "tracer_core",
                    "--profile",
                    "release_bundle",
                    "--runtime-platform",
                    "windows",
                ]
            )

        self.assertEqual(FakeBuildCommand.last_kwargs["runtime_platform"], "windows")

    def test_build_rejects_tracer_windows_rust_cli_without_runtime_platform(self):
        stdout = io.StringIO()
        stderr = io.StringIO()
        with patch.object(
            sys,
            "argv",
            ["run.py", "build", "--app", "tracer_windows_rust_cli", "--profile", "release_bundle"],
        ), redirect_stdout(stdout), redirect_stderr(stderr):
            rc = self.run_module.main()

        self.assertEqual(rc, 2)
        self.assertIn("--runtime-platform windows", stderr.getvalue())
        self.assertEqual(stdout.getvalue(), "")

    def test_build_rejects_build_dir_for_tracer_android(self):
        stdout = io.StringIO()
        stderr = io.StringIO()
        with patch.object(
            sys,
            "argv",
            ["run.py", "build", "--app", "tracer_android", "--build-dir", "build_fast"],
        ), redirect_stdout(stdout), redirect_stderr(stderr):
            rc = self.run_module.main()

        self.assertEqual(rc, 2)
        self.assertIn("does not support `--build-dir`", stderr.getvalue())
        self.assertIn("tracer_android", stderr.getvalue())
        self.assertEqual(stdout.getvalue(), "")

    def test_verify_rejects_build_dir_for_tracer_android(self):
        stdout = io.StringIO()
        stderr = io.StringIO()
        with patch.object(
            sys,
            "argv",
            ["run.py", "verify", "--app", "tracer_android", "--build-dir", "build_fast"],
        ), redirect_stdout(stdout), redirect_stderr(stderr):
            rc = self.run_module.main()

        self.assertEqual(rc, 2)
        self.assertIn("does not support `--build-dir`", stderr.getvalue())
        self.assertIn("tracer_android", stderr.getvalue())
        self.assertEqual(stdout.getvalue(), "")

    def test_verify_repeated_profiles_rejected_for_non_gradle_app(self):
        stdout = io.StringIO()
        stderr = io.StringIO()
        with patch.object(
            sys,
            "argv",
            [
                "run.py",
                "verify",
                "--app",
                "tracer_core",
                "--profile",
                "fast",
                "--profile",
                "release_bundle",
            ],
        ), redirect_stdout(stdout), redirect_stderr(stderr):
            rc = self.run_module.main()

        self.assertEqual(rc, 2)
        self.assertIn("supported only for Gradle-backed apps", stderr.getvalue())
        self.assertEqual(stdout.getvalue(), "")

    def test_verify_rejects_removed_quick_flag(self):
        stderr = io.StringIO()
        with patch.object(
            sys,
            "argv",
            ["run.py", "verify", "--app", "tracer_core", "--quick"],
        ), redirect_stderr(stderr), self.assertRaises(SystemExit) as raised:
            self.run_module.main()

        self.assertEqual(raised.exception.code, 2)
        self.assertIn("unrecognized arguments: --quick", stderr.getvalue())
