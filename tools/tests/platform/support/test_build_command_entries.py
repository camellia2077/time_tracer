import io
from contextlib import redirect_stdout
from pathlib import Path
from unittest import TestCase

from tools.toolchain.commands.cmd_build.command_entries import _print_build_output_dir
from tools.toolchain.core.context import Context


class _FakeBuildCommand:
    def __init__(self, repo_root: Path) -> None:
        self.ctx = Context(repo_root)


class TestBuildCommandEntries(TestCase):
    def test_tracer_android_edit_reports_debug_apk_output_dir(self) -> None:
        repo_root = Path(__file__).resolve().parents[4]
        command = _FakeBuildCommand(repo_root)
        stdout = io.StringIO()

        with redirect_stdout(stdout):
            _print_build_output_dir(
                command=command,
                app_name="tracer_android",
                backend="gradle",
                build_dir_name="build",
                tidy=False,
                profile_name="android_edit",
            )

        self.assertIn(
            "apps/android/app/build/outputs/apk/debug",
            stdout.getvalue(),
        )

    def test_tracer_android_release_reports_final_release_apk_output_dir(self) -> None:
        repo_root = Path(__file__).resolve().parents[4]
        command = _FakeBuildCommand(repo_root)
        stdout = io.StringIO()

        with redirect_stdout(stdout):
            _print_build_output_dir(
                command=command,
                app_name="tracer_android",
                backend="gradle",
                build_dir_name="build",
                tidy=False,
                profile_name="android_release",
            )

        self.assertIn(
            "apps/android/app/build/outputs/final-apk/release",
            stdout.getvalue(),
        )

    def test_tracer_android_device_profile_reports_install_success(self) -> None:
        repo_root = Path(__file__).resolve().parents[4]
        command = _FakeBuildCommand(repo_root)
        stdout = io.StringIO()

        with redirect_stdout(stdout):
            _print_build_output_dir(
                command=command,
                app_name="tracer_android",
                backend="gradle",
                build_dir_name="build",
                tidy=False,
                profile_name="android_edit_device",
            )

        self.assertIn("Android app installed successfully.", stdout.getvalue())
