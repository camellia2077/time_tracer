import argparse
import subprocess
import tempfile
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.cli.handlers import android


class _FakeContext:
    def __init__(self, root: Path, app_dir: Path) -> None:
        self.repo_root = root
        self._app_dir = app_dir

    def get_app_dir(self, _app_name: str) -> Path:
        return self._app_dir

    def setup_env(self) -> dict[str, str]:
        return {"PATH": "test-path"}


class _FakeBuildCommand:
    last_profile = None

    def __init__(self, _ctx) -> None:
        pass

    def build(self, **kwargs) -> int:
        _FakeBuildCommand.last_profile = kwargs["profile_name"]
        return 0


class TestAndroidCommand(TestCase):
    def test_debug_build_only(self) -> None:
        args = argparse.Namespace(
            variant="debug",
            install=False,
            install_only=False,
            serial=None,
            with_test_data=False,
            keep_database=False,
            rebuild_database=False,
        )

        with patch.object(android, "BuildCommand", _FakeBuildCommand):
            result = android.run(args, _FakeContext(Path("."), Path(".")))

        self.assertEqual(result, 0)
        self.assertEqual(_FakeBuildCommand.last_profile, "android_edit")

    def test_debug_build_install_and_injects_test_data_without_rebuilding_database(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            app_dir = root / "apps/android"
            apk = app_dir / "app/build/outputs/apk/debug/app-debug.apk"
            apk.parent.mkdir(parents=True)
            apk.write_bytes(b"apk")
            context = _FakeContext(root, app_dir)
            args = argparse.Namespace(
                variant="debug",
                install=True,
                install_only=False,
                serial="device-1",
                with_test_data=True,
                keep_database=False,
                rebuild_database=False,
            )

            with (
                patch.object(android, "BuildCommand", _FakeBuildCommand),
                patch.object(android.shutil, "which", return_value="adb.exe"),
                patch.object(
                    android.subprocess,
                    "run",
                    side_effect=[
                        subprocess.CompletedProcess([], 0),
                        subprocess.CompletedProcess([], 0),
                        subprocess.CompletedProcess([], 0),
                    ],
                ) as run_mock,
            ):
                result = android.run(args, context)

            self.assertEqual(result, 0)
            self.assertEqual(_FakeBuildCommand.last_profile, "android_edit")
            self.assertEqual(run_mock.call_count, 3)
            self.assertIn("install", run_mock.call_args_list[0].args[0])
            self.assertIn("push_test_data.py", " ".join(run_mock.call_args_list[1].args[0]))
            self.assertIn("monkey", run_mock.call_args_list[2].args[0])
            self.assertNotIn(
                "REBUILD_DATABASE",
                " ".join(" ".join(call.args[0]) for call in run_mock.call_args_list),
            )

    def test_debug_install_and_rebuilds_database_when_explicitly_requested(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            app_dir = root / "apps/android"
            apk = app_dir / "app/build/outputs/apk/debug/app-debug.apk"
            apk.parent.mkdir(parents=True)
            apk.write_bytes(b"apk")
            args = argparse.Namespace(
                variant="debug",
                install=True,
                install_only=False,
                serial="device-1",
                with_test_data=False,
                keep_database=False,
                rebuild_database=True,
            )

            with (
                patch.object(android, "BuildCommand", _FakeBuildCommand),
                patch.object(android.shutil, "which", return_value="adb.exe"),
                patch.object(
                    android.subprocess,
                    "run",
                    side_effect=[
                        subprocess.CompletedProcess([], 0),
                        subprocess.CompletedProcess(
                            [], 0, stdout="Broadcast completed: result=0"
                        ),
                        subprocess.CompletedProcess([], 0),
                    ],
                ) as run_mock,
            ):
                result = android.run(args, _FakeContext(root, app_dir))

            self.assertEqual(result, 0)
            self.assertEqual(run_mock.call_count, 3)
            self.assertIn("install", run_mock.call_args_list[0].args[0])
            self.assertIn("REBUILD_DATABASE", " ".join(run_mock.call_args_list[1].args[0]))
            self.assertIn("monkey", run_mock.call_args_list[2].args[0])

    def test_debug_install_only_does_not_build(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            app_dir = root / "apps/android"
            apk = app_dir / "app/build/outputs/apk/debug/app-debug.apk"
            apk.parent.mkdir(parents=True)
            apk.write_bytes(b"apk")
            args = argparse.Namespace(
                variant="debug",
                install=True,
                install_only=True,
                serial="device-1",
                with_test_data=False,
                keep_database=False,
                rebuild_database=False,
            )

            with (
                patch.object(android, "BuildCommand", _FakeBuildCommand),
                patch.object(android.shutil, "which", return_value="adb.exe"),
                patch.object(
                    android.subprocess,
                    "run",
                    return_value=subprocess.CompletedProcess([], 0),
                ) as run_mock,
            ):
                result = android.run(args, _FakeContext(root, app_dir))

            self.assertEqual(result, 0)
            self.assertEqual(run_mock.call_count, 1)

    def test_release_rejects_test_data(self) -> None:
        args = argparse.Namespace(
            variant="release",
            install=True,
            install_only=False,
            serial="device-1",
            with_test_data=True,
            keep_database=False,
            rebuild_database=False,
        )

        result = android.run(args, _FakeContext(Path("."), Path(".")))

        self.assertEqual(result, 2)
