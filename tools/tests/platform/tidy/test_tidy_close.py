import json
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.tidy.execution.close import TidyCloseCommand
from tools.toolchain.commands.tidy.workspace import ResolvedTidyWorkspace
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[4]


def _write_task_json(task_path: Path) -> None:
    task_path.parent.mkdir(parents=True, exist_ok=True)
    task_path.write_text(
        json.dumps(
            {
                "version": 4,
                "task_id": "001",
                "cluster_id": task_path.parent.name,
                "scan_id": "scan_fixture",
                "source_file": "C:/code/time_tracer/libs/tracer_core/src/example.cpp",
                "workspace": "build_tidy_core_family",
                "source_scope": "core_family",
                "compiler_errors": False,
                "checks": ["readability-identifier-naming"],
                "diagnostics": [{"line": 7, "col": 4, "severity": "warning", "check": "readability-identifier-naming", "message": "example diagnostic"}],
            }
        ),
        encoding="utf-8",
    )


class TestTidyCloseCommand(TestCase):
    def _workspace(self, root: Path, tasks_dir: Path) -> ResolvedTidyWorkspace:
        return ResolvedTidyWorkspace(
            source_scope="core_family",
            build_dir_name="build_tidy_core_family",
            source_roots=[],
            prebuild_targets=[],
        )

    def test_dry_run_previews_final_full_without_verify_or_queue_changes(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            _write_task_json(tasks_dir / "clusters" / "cluster_example" / "task_001.json")
            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=tasks_dir,
                tidy_state_path=root / "tidy_state.json",
                tidy_result_path=root / "tidy_result.json",
            )
            statuses: list[str] = []
            with (
                patch("tools.toolchain.commands.tidy.execution.close.tidy_workspace.resolve_workspace", return_value=self._workspace(root, tasks_dir)),
                patch("tools.toolchain.commands.tidy.execution.close.TidyRefreshCommand.execute", return_value=0) as refresh,
                patch("tools.toolchain.commands.tidy.execution.close.VerifyCommand.execute", return_value=0) as verify,
                patch("tools.toolchain.commands.tidy.execution.close.tidy_result_summary.write_tidy_result", side_effect=lambda **kwargs: statuses.append(str(kwargs["status"]))),
            ):
                result = TidyCloseCommand(ctx).execute(app_name="tracer_core_shell", tidy_build_dir_name="build_tidy_core_family", dry_run=True)
            self.assertEqual(result, 0)
            self.assertEqual(statuses, ["dry_run"])
            self.assertTrue(refresh.call_args.kwargs["dry_run"])
            verify.assert_not_called()
            self.assertTrue((tasks_dir / "clusters" / "cluster_example" / "task_001.json").exists())

    def test_tidy_only_and_full_verify_gates_are_distinct(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=tasks_dir,
                tidy_state_path=root / "tidy_state.json",
                tidy_result_path=root / "tidy_result.json",
            )
            statuses: list[str] = []
            with (
                patch("tools.toolchain.commands.tidy.execution.close.tidy_workspace.resolve_workspace", return_value=self._workspace(root, tasks_dir)),
                patch("tools.toolchain.commands.tidy.execution.close.TidyRefreshCommand.execute", return_value=0) as refresh,
                patch("tools.toolchain.commands.tidy.execution.close.VerifyCommand.execute", return_value=0) as verify,
                patch("tools.toolchain.commands.tidy.execution.close.tidy_state.update_state", return_value=root / "tidy_state.json"),
                patch("tools.toolchain.commands.tidy.execution.close.tidy_result_summary.write_tidy_result", side_effect=lambda **kwargs: statuses.append(str(kwargs["status"]))),
            ):
                tidy_only = TidyCloseCommand(ctx).execute(app_name="tracer_core_shell", tidy_build_dir_name="build_tidy_core_family", tidy_only=True)
                full = TidyCloseCommand(ctx).execute(app_name="tracer_core_shell", tidy_build_dir_name="build_tidy_core_family")
            self.assertEqual((tidy_only, full), (0, 0))
            self.assertEqual(refresh.call_count, 2)
            verify.assert_called_once()
            self.assertEqual(statuses, ["completed_tidy_only", "completed"])

    def test_pending_after_final_full_stops_without_auto_draining(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=tasks_dir,
                tidy_state_path=root / "tidy_state.json",
                tidy_result_path=root / "tidy_result.json",
            )
            statuses: list[str] = []
            with (
                patch("tools.toolchain.commands.tidy.execution.close.tidy_workspace.resolve_workspace", return_value=self._workspace(root, tasks_dir)),
                patch("tools.toolchain.commands.tidy.execution.close.TidyRefreshCommand.execute", side_effect=lambda **_kwargs: (_write_task_json(tasks_dir / "clusters" / "cluster_example" / "task_001.json") or 0)),
                patch("tools.toolchain.commands.tidy.execution.close.VerifyCommand.execute", return_value=0),
                patch("tools.toolchain.commands.tidy.execution.close.tidy_result_summary.write_tidy_result", side_effect=lambda **kwargs: statuses.append(str(kwargs["status"]))),
            ):
                result = TidyCloseCommand(ctx).execute(app_name="tracer_core_shell", tidy_build_dir_name="build_tidy_core_family")
            self.assertEqual(result, 1)
            self.assertEqual(statuses, ["pending_after_final_full"])
