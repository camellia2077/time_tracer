import importlib.util
import io
import sys
from contextlib import redirect_stderr
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch


def _load_run_module():
    repo_root = Path(__file__).resolve().parents[3]
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

    def _assert_return_zero(self, argv: list[str]) -> None:
        with patch.object(sys, "argv", argv):
            rc = self.run_module.main()
        self.assertEqual(rc, 0)

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

        with patch("toolchain.cli.handlers.build.BuildCommand", FakeBuildCommand):
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
        self.assertEqual(FakeBuildCommand.last_args[0], "tracer_core")
        self.assertFalse(FakeBuildCommand.last_args[1])
        self.assertEqual(FakeBuildCommand.last_args[2], [])
        self.assertEqual(FakeBuildCommand.last_kwargs["profile_name"], "fast")
        self.assertEqual(FakeBuildCommand.last_kwargs["cmake_args"], ["-DA=1", "-DB=2"])

    def test_post_change_uses_default_build_dir_without_profile(self):
        class FakePostChangeCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakePostChangeCommand.last_kwargs = kwargs
                return 0

        with patch("toolchain.cli.handlers.post_change.PostChangeCommand", FakePostChangeCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "post-change",
                    "--app",
                    "tracer_core",
                    "--dry-run",
                ]
            )

        self.assertIsNotNone(FakePostChangeCommand.last_kwargs)
        self.assertEqual(FakePostChangeCommand.last_kwargs["app_name"], "tracer_core")
        self.assertEqual(FakePostChangeCommand.last_kwargs["profile_name"], None)
        self.assertEqual(FakePostChangeCommand.last_kwargs["build_dir_name"], "build_fast")

    def test_post_change_keeps_build_dir_unset_when_profile_is_given(self):
        class FakePostChangeCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakePostChangeCommand.last_kwargs = kwargs
                return 0

        with patch("toolchain.cli.handlers.post_change.PostChangeCommand", FakePostChangeCommand):
            self._assert_return_zero(
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
        self.assertEqual(FakePostChangeCommand.last_kwargs["profile_name"], "fast")
        self.assertIsNone(FakePostChangeCommand.last_kwargs["build_dir_name"])

    def test_post_change_tracer_android_keeps_build_dir_unset(self):
        class FakePostChangeCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakePostChangeCommand.last_kwargs = kwargs
                return 0

        with patch("toolchain.cli.handlers.post_change.PostChangeCommand", FakePostChangeCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "post-change",
                    "--app",
                    "tracer_android",
                    "--dry-run",
                ]
            )

        self.assertIsNotNone(FakePostChangeCommand.last_kwargs)
        self.assertEqual(FakePostChangeCommand.last_kwargs["app_name"], "tracer_android")
        self.assertIsNone(FakePostChangeCommand.last_kwargs["build_dir_name"])

    def test_post_change_dispatches_cmake_args(self):
        class FakePostChangeCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakePostChangeCommand.last_kwargs = kwargs
                return 0

        with patch("toolchain.cli.handlers.post_change.PostChangeCommand", FakePostChangeCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "post-change",
                    "--app",
                    "tracer_core",
                    "--dry-run",
                    "--cmake-args=-DA=1",
                    "--cmake-args=-DB=2",
                ]
            )

        self.assertIsNotNone(FakePostChangeCommand.last_kwargs)
        self.assertEqual(FakePostChangeCommand.last_kwargs["cmake_args"], ["-DA=1", "-DB=2"])

    def test_unrecognized_arguments_show_build_verify_hint(self):
        stderr = io.StringIO()
        with patch.object(sys, "argv", ["run.py", "post-change", "--bad-arg"]), redirect_stderr(
            stderr
        ), self.assertRaises(SystemExit) as raised:
            self.run_module.main()

        self.assertEqual(raised.exception.code, 2)
        error_text = stderr.getvalue()
        self.assertIn("unrecognized arguments: --bad-arg", error_text)
        self.assertIn("python scripts/run.py post-change -h", error_text)
        self.assertIn("python scripts/run.py build ...", error_text)
        self.assertIn("python scripts/run.py verify ...", error_text)

    def test_config_migrate_defaults_to_dry_run(self):
        class FakeConfigMigrateCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeConfigMigrateCommand.last_kwargs = kwargs
                return 0

        with patch(
            "toolchain.cli.handlers.config_migrate.ConfigMigrateCommand",
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
