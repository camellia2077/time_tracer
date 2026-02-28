import importlib.util
import sys
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch


def _load_run_module():
    repo_root = Path(__file__).resolve().parents[2]
    run_py = repo_root / "scripts" / "run.py"
    spec = importlib.util.spec_from_file_location("scripts_run_module", run_py)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Failed to load module spec from {run_py}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TestRunCliDispatch(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.run_module = _load_run_module()

    def _assert_exit_zero(self, argv: list[str]) -> None:
        with patch.object(sys, "argv", argv):
            with self.assertRaises(SystemExit) as exit_info:
                self.run_module.main()
        self.assertEqual(exit_info.exception.code, 0)

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

        with patch.object(self.run_module, "BuildCommand", FakeBuildCommand):
            self._assert_exit_zero(
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
        args = FakeBuildCommand.last_args
        kwargs = FakeBuildCommand.last_kwargs
        self.assertEqual(args[0], "tracer_core")
        self.assertFalse(args[1])
        self.assertEqual(args[2], [])
        self.assertEqual(kwargs["profile_name"], "fast")
        self.assertEqual(kwargs["cmake_args"], ["-DA=1", "-DB=2"])

    def test_post_change_uses_default_build_dir_without_profile(self):
        class FakePostChangeCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakePostChangeCommand.last_kwargs = kwargs
                return 0

        with patch.object(self.run_module, "PostChangeCommand", FakePostChangeCommand):
            self._assert_exit_zero(
                [
                    "run.py",
                    "post-change",
                    "--app",
                    "tracer_core",
                    "--dry-run",
                ]
            )

        self.assertIsNotNone(FakePostChangeCommand.last_kwargs)
        kwargs = FakePostChangeCommand.last_kwargs
        self.assertEqual(kwargs["app_name"], "tracer_core")
        self.assertEqual(kwargs["profile_name"], None)
        self.assertEqual(kwargs["build_dir_name"], "build_fast")

    def test_post_change_keeps_build_dir_unset_when_profile_is_given(self):
        class FakePostChangeCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakePostChangeCommand.last_kwargs = kwargs
                return 0

        with patch.object(self.run_module, "PostChangeCommand", FakePostChangeCommand):
            self._assert_exit_zero(
                [
                    "run.py",
                    "post-change",
                    "--app",
                    "tracer_core",
                    "--profile",
                    "fast",
                    "--dry-run",
                ]
            )

        self.assertIsNotNone(FakePostChangeCommand.last_kwargs)
        kwargs = FakePostChangeCommand.last_kwargs
        self.assertEqual(kwargs["profile_name"], "fast")
        self.assertIsNone(kwargs["build_dir_name"])

    def test_post_change_tracer_android_keeps_build_dir_unset(self):
        class FakePostChangeCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakePostChangeCommand.last_kwargs = kwargs
                return 0

        with patch.object(self.run_module, "PostChangeCommand", FakePostChangeCommand):
            self._assert_exit_zero(
                [
                    "run.py",
                    "post-change",
                    "--app",
                    "tracer_android",
                    "--dry-run",
                ]
            )

        self.assertIsNotNone(FakePostChangeCommand.last_kwargs)
        kwargs = FakePostChangeCommand.last_kwargs
        self.assertEqual(kwargs["app_name"], "tracer_android")
        self.assertIsNone(kwargs["build_dir_name"])

    def test_config_migrate_defaults_to_dry_run(self):
        class FakeConfigMigrateCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeConfigMigrateCommand.last_kwargs = kwargs
                return 0

        with patch.object(self.run_module, "ConfigMigrateCommand", FakeConfigMigrateCommand):
            self._assert_exit_zero(
                [
                    "run.py",
                    "config-migrate",
                    "--app",
                    "tracer_windows_cli",
                ]
            )

        self.assertIsNotNone(FakeConfigMigrateCommand.last_kwargs)
        kwargs = FakeConfigMigrateCommand.last_kwargs
        self.assertEqual(kwargs["app_name"], "tracer_windows_cli")
        self.assertTrue(kwargs["dry_run"])
        self.assertFalse(kwargs["apply_changes"])
        self.assertFalse(kwargs["rollback"])
