import importlib.util
import io
import argparse
import sys
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.cli.dispatch import dispatch_command  # noqa: E402
from tools.toolchain.core.context import Context  # noqa: E402


def _load_run_module():
    run_py = REPO_ROOT / "tools" / "run.py"
    spec = importlib.util.spec_from_file_location("tools_run_module", run_py)
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

    def test_validate_dispatches_plan_paths_and_run_name(self):
        class FakeValidateCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeValidateCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.validate.ValidateCommand", FakeValidateCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "validate",
                    "--plan",
                    "temp/import_batch01.toml",
                    "--paths",
                    "libs/tracer_core/src/a.cpp",
                    "libs/tracer_core/src/b.cpp",
                    "--paths-file",
                    "temp/import_batch01.paths",
                    "--run-name",
                    "batch01",
                ]
            )

        self.assertIsNotNone(FakeValidateCommand.last_kwargs)
        self.assertEqual(FakeValidateCommand.last_kwargs["plan_path"], "temp/import_batch01.toml")
        self.assertEqual(
            FakeValidateCommand.last_kwargs["raw_paths"],
            ["libs/tracer_core/src/a.cpp", "libs/tracer_core/src/b.cpp"],
        )
        self.assertEqual(FakeValidateCommand.last_kwargs["paths_file"], "temp/import_batch01.paths")
        self.assertEqual(FakeValidateCommand.last_kwargs["run_name"], "batch01")
        self.assertTrue(FakeValidateCommand.last_kwargs["verbose"])

    def test_validate_dispatches_quiet_flag(self):
        class FakeValidateCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeValidateCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.validate.ValidateCommand", FakeValidateCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "validate",
                    "--plan",
                    "temp/import_batch01.toml",
                    "--paths",
                    "libs/tracer_core/src/a.cpp",
                    "--quiet",
                ]
            )

        self.assertIsNotNone(FakeValidateCommand.last_kwargs)
        self.assertFalse(FakeValidateCommand.last_kwargs["verbose"])

    def test_tidy_dispatches_source_scope_and_build_dir(self):
        class FakeTidyCommand:
            last_args = None
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_args = args
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy",
                    "--app",
                    "tracer_core_shell",
                    "--source-scope",
                    "core_family",
                    "--build-dir",
                    "build_tidy_core_family",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_args, ("tracer_core_shell", []))
        self.assertEqual(FakeTidyCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(FakeTidyCommand.last_kwargs["build_dir_name"], "build_tidy_core_family")
        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "text")

    def test_analyze_dispatches_source_scope_build_dir_and_profile(self):
        class FakeAnalyzeCommand:
            last_args = None
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeAnalyzeCommand.last_args = args
                FakeAnalyzeCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.analyze.AnalyzeCommand", FakeAnalyzeCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "analyze",
                    "--app",
                    "tracer_core_shell",
                    "--source-scope",
                    "core_family",
                    "--build-dir",
                    "build_analyze_core_family",
                    "--profile",
                    "fast",
                    "--jobs",
                    "8",
                ]
            )

        self.assertEqual(FakeAnalyzeCommand.last_args, ("tracer_core_shell",))
        self.assertEqual(FakeAnalyzeCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(
            FakeAnalyzeCommand.last_kwargs["build_dir_name"],
            "build_analyze_core_family",
        )
        self.assertEqual(FakeAnalyzeCommand.last_kwargs["profile_name"], "fast")
        self.assertEqual(FakeAnalyzeCommand.last_kwargs["jobs"], 8)

    def test_analyze_split_dispatches_scope_build_dir_and_batch_size(self):
        class FakeAnalyzeCommand:
            last_args = None
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def split_only(self, *args, **kwargs):
                FakeAnalyzeCommand.last_args = args
                FakeAnalyzeCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.analyze_split.AnalyzeCommand",
            FakeAnalyzeCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "analyze-split",
                    "--app",
                    "tracer_core_shell",
                    "--source-scope",
                    "core_family",
                    "--build-dir",
                    "build_analyze_core_family",
                    "--batch-size",
                    "7",
                ]
            )

        self.assertEqual(FakeAnalyzeCommand.last_args, ("tracer_core_shell",))
        self.assertEqual(FakeAnalyzeCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(
            FakeAnalyzeCommand.last_kwargs["build_dir_name"],
            "build_analyze_core_family",
        )
        self.assertEqual(FakeAnalyzeCommand.last_kwargs["batch_size"], 7)

    def test_tidy_dispatches_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy",
                    "--app",
                    "tracer_core_shell",
                    "--task-view",
                    "text+toon",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "text+toon")

    def test_tidy_dispatches_toon_only_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy",
                    "--app",
                    "tracer_core_shell",
                    "--task-view",
                    "toon",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "toon")

    def test_tidy_dispatches_json_only_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy",
                    "--app",
                    "tracer_core_shell",
                    "--task-view",
                    "json",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "json")

    def test_tidy_split_dispatches_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def split_only(self, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy_split.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-split",
                    "--app",
                    "tracer_core_shell",
                    "--task-view",
                    "text+toon",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "text+toon")

    def test_tidy_split_dispatches_toon_only_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def split_only(self, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy_split.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-split",
                    "--app",
                    "tracer_core_shell",
                    "--task-view",
                    "toon",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "toon")

    def test_tidy_split_dispatches_json_only_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def split_only(self, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy_split.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-split",
                    "--app",
                    "tracer_core_shell",
                    "--task-view",
                    "json",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "json")

    def test_tidy_fix_dispatches_source_scope_and_tidy_build_dir(self):
        class FakeTidyFixCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyFixCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_fix.TidyFixCommand",
            FakeTidyFixCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-fix",
                    "--app",
                    "tracer_core_shell",
                    "--source-scope",
                    "core_family",
                    "--tidy-build-dir",
                    "build_tidy_core_family",
                ]
            )

        self.assertEqual(FakeTidyFixCommand.last_kwargs["app_name"], "tracer_core_shell")
        self.assertEqual(FakeTidyFixCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(
            FakeTidyFixCommand.last_kwargs["tidy_build_dir_name"],
            "build_tidy_core_family",
        )

    def test_clean_dispatches_tidy_build_dir(self):
        class FakeCleanCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeCleanCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.clean.CleanCommand", FakeCleanCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "clean",
                    "--app",
                    "tracer_core_shell",
                    "--tidy-build-dir",
                    "build_tidy_core_family",
                    "001",
                ]
            )

        self.assertEqual(FakeCleanCommand.last_kwargs["app_name"], "tracer_core_shell")
        self.assertEqual(
            FakeCleanCommand.last_kwargs["tidy_build_dir_name"],
            "build_tidy_core_family",
        )
        self.assertEqual(FakeCleanCommand.last_kwargs["task_ids"], ["001"])

    def test_tidy_batch_dispatches_scope_and_tidy_build_dir(self):
        class FakeTidyBatchCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyBatchCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_batch.TidyBatchCommand",
            FakeTidyBatchCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-batch",
                    "--app",
                    "tracer_core_shell",
                    "--batch-id",
                    "001",
                    "--preset",
                    "sop",
                    "--source-scope",
                    "core_family",
                    "--tidy-build-dir",
                    "build_tidy_core_family",
                ]
            )

        self.assertEqual(FakeTidyBatchCommand.last_kwargs["app_name"], "tracer_core_shell")
        self.assertEqual(FakeTidyBatchCommand.last_kwargs["batch_id"], "001")
        self.assertEqual(FakeTidyBatchCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(
            FakeTidyBatchCommand.last_kwargs["tidy_build_dir_name"],
            "build_tidy_core_family",
        )

    def test_tidy_flow_dispatches_scope_and_tidy_build_dir(self):
        class FakeTidyFlowCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyFlowCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_flow.TidyFlowCommand",
            FakeTidyFlowCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-flow",
                    "--app",
                    "tracer_core_shell",
                    "--all",
                    "--source-scope",
                    "core_family",
                    "--tidy-build-dir",
                    "build_tidy_core_family",
                ]
            )

        self.assertEqual(FakeTidyFlowCommand.last_kwargs["app_name"], "tracer_core_shell")
        self.assertTrue(FakeTidyFlowCommand.last_kwargs["process_all"])
        self.assertEqual(FakeTidyFlowCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(
            FakeTidyFlowCommand.last_kwargs["tidy_build_dir_name"],
            "build_tidy_core_family",
        )

    def test_tidy_task_fix_dispatches_task_selector_and_scope(self):
        class FakeTidyTaskFixCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyTaskFixCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_task_fix.TidyTaskFixCommand",
            FakeTidyTaskFixCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-task-fix",
                    "--app",
                    "tracer_core_shell",
                    "--batch-id",
                    "002",
                    "--task-id",
                    "011",
                    "--source-scope",
                    "core_family",
                    "--tidy-build-dir",
                    "build_tidy_core_family",
                    "--dry-run",
                ]
            )

        self.assertEqual(FakeTidyTaskFixCommand.last_kwargs["app_name"], "tracer_core_shell")
        self.assertEqual(FakeTidyTaskFixCommand.last_kwargs["batch_id"], "002")
        self.assertEqual(FakeTidyTaskFixCommand.last_kwargs["task_id"], "011")
        self.assertEqual(FakeTidyTaskFixCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(
            FakeTidyTaskFixCommand.last_kwargs["tidy_build_dir_name"],
            "build_tidy_core_family",
        )
        self.assertTrue(FakeTidyTaskFixCommand.last_kwargs["dry_run"])

    def test_tidy_task_patch_dispatches_task_log(self):
        class FakeTidyTaskPatchCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyTaskPatchCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_task_patch.TidyTaskPatchCommand",
            FakeTidyTaskPatchCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-task-patch",
                    "--app",
                    "tracer_core_shell",
                    "--task-log",
                    "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.log",
                ]
            )

        self.assertEqual(FakeTidyTaskPatchCommand.last_kwargs["app_name"], "tracer_core_shell")
        self.assertEqual(
            FakeTidyTaskPatchCommand.last_kwargs["task_log_path"],
            "out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.log",
        )

    def test_tidy_task_suggest_dispatches_task_selector(self):
        class FakeTidyTaskSuggestCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyTaskSuggestCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_task_suggest.TidyTaskSuggestCommand",
            FakeTidyTaskSuggestCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-task-suggest",
                    "--app",
                    "tracer_core_shell",
                    "--batch-id",
                    "002",
                    "--task-id",
                    "011",
                    "--tidy-build-dir",
                    "build_tidy_core_family",
                ]
            )

        self.assertEqual(
            FakeTidyTaskSuggestCommand.last_kwargs["tidy_build_dir_name"],
            "build_tidy_core_family",
        )
        self.assertEqual(FakeTidyTaskSuggestCommand.last_kwargs["task_id"], "011")

    def test_tidy_step_dispatches_verify_related_flags(self):
        class FakeTidyStepCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyStepCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.cli.handlers.tidy.tidy_step.TidyStepCommand",
            FakeTidyStepCommand,
        ):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy-step",
                    "--app",
                    "tracer_core_shell",
                    "--batch-id",
                    "002",
                    "--task-id",
                    "011",
                    "--build-dir",
                    "build_fast",
                    "--profile",
                    "fast",
                    "--concise",
                    "--kill-build-procs",
                ]
            )

        self.assertEqual(FakeTidyStepCommand.last_kwargs["verify_build_dir_name"], "build_fast")
        self.assertEqual(FakeTidyStepCommand.last_kwargs["profile_name"], "fast")
        self.assertTrue(FakeTidyStepCommand.last_kwargs["concise"])
        self.assertTrue(FakeTidyStepCommand.last_kwargs["kill_build_procs"])

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

    def test_tidy_dispatches_profile_concise_and_kill_build_procs(self):
        class FakeTidyCommand:
            last_args = None
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_args = args
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py",
                    "tidy",
                    "--app",
                    "tracer_core_shell",
                    "--profile",
                    "fast",
                    "--build-dir",
                    "build_tidy_core_family",
                    "--concise",
                    "--kill-build-procs",
                    "--source-scope",
                    "core_family",
                ]
            )

        self.assertEqual(FakeTidyCommand.last_args, ("tracer_core_shell", []))
        self.assertEqual(FakeTidyCommand.last_kwargs["profile_name"], "fast")
        self.assertEqual(FakeTidyCommand.last_kwargs["build_dir_name"], "build_tidy_core_family")
        self.assertTrue(FakeTidyCommand.last_kwargs["concise"])
        self.assertTrue(FakeTidyCommand.last_kwargs["kill_build_procs"])

    def test_unrecognized_arguments_show_build_verify_hint(self):
        stderr = io.StringIO()
        with patch.object(
            sys,
            "argv",
            ["run.py", "validate", "--plan", "temp/import_batch01.toml", "--bad-arg"],
        ), redirect_stderr(stderr), self.assertRaises(SystemExit) as raised:
            self.run_module.main()

        self.assertEqual(raised.exception.code, 2)
        error_text = stderr.getvalue()
        self.assertIn("unrecognized arguments: --bad-arg", error_text)
        self.assertIn("python tools/run.py validate -h", error_text)
        self.assertIn("python tools/run.py build ...", error_text)
        self.assertIn("python tools/run.py verify ...", error_text)
        self.assertIn("python tools/run.py validate ...", error_text)

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
