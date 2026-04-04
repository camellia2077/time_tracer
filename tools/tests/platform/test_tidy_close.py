import json
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.tidy.close import TidyCloseCommand
from tools.toolchain.commands.tidy.workspace import ResolvedTidyWorkspace
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[3]


def _write_task_json(
    task_path: Path,
    *,
    task_id: str = "001",
    batch_id: str = "batch_001",
    source_file: str = "C:/code/time_tracer/libs/tracer_core/src/example.cpp",
    checks: list[str] | None = None,
) -> None:
    task_path.parent.mkdir(parents=True, exist_ok=True)
    task_path.write_text(
        json.dumps(
            {
                "version": 2,
                "task_id": task_id,
                "batch_id": batch_id,
                "queue_batch_id": batch_id,
                "source_file": source_file,
                "workspace": "build_tidy_core_family",
                "source_scope": "core_family",
                "compiler_errors": False,
                "checks": checks or ["readability-identifier-naming"],
                "diagnostics": [
                    {
                        "line": 7,
                        "col": 4,
                        "severity": "warning",
                        "check": (checks or ["readability-identifier-naming"])[0],
                        "message": "example diagnostic",
                    }
                ],
            },
            indent=2,
        ),
        encoding="utf-8",
    )


class TestTidyCloseCommand(TestCase):
    def test_execute_without_stabilize_preserves_single_round_behavior(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            _write_task_json(tasks_dir / "batch_001" / "task_001.json")

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=tasks_dir,
                batch_state_path=root / "batch_state.json",
                tidy_result_path=root / "tidy_result.json",
            )
            workspace = ResolvedTidyWorkspace(
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
                source_roots=[],
                prebuild_targets=[],
            )

            statuses: list[str] = []

            with (
                patch(
                    "tools.toolchain.commands.tidy.close.tidy_workspace.resolve_workspace",
                    return_value=workspace,
                ),
                patch(
                    "tools.toolchain.commands.tidy.close.TidyRefreshCommand.execute",
                    return_value=0,
                ),
                patch(
                    "tools.toolchain.commands.tidy.close.VerifyCommand.execute",
                    return_value=0,
                ),
                patch(
                    "tools.toolchain.commands.tidy.close.tidy_result_summary.write_tidy_result",
                    side_effect=lambda **kwargs: statuses.append(str(kwargs["status"])),
                ),
                patch(
                    "tools.toolchain.commands.tidy.close.TidyStepCommand.execute",
                ) as step_execute,
            ):
                ret = TidyCloseCommand(ctx).execute(
                    app_name="tracer_core_shell",
                    source_scope="core_family",
                    tidy_build_dir_name="build_tidy_core_family",
                )

            self.assertEqual(ret, 1)
            self.assertEqual(statuses, ["pending_tasks"])
            step_execute.assert_not_called()

    def test_execute_with_stabilize_drains_repopulated_tasks_and_retries(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=tasks_dir,
                batch_state_path=root / "batch_state.json",
                tidy_result_path=root / "tidy_result.json",
            )
            workspace = ResolvedTidyWorkspace(
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
                source_roots=[],
                prebuild_targets=[],
            )

            statuses: list[str] = []
            refresh_calls = {"count": 0}

            def _refresh(**_kwargs):
                refresh_calls["count"] += 1
                if refresh_calls["count"] == 1:
                    _write_task_json(tasks_dir / "batch_001" / "task_001.json")
                return 0

            def _step(**kwargs):
                Path(kwargs["task_log_path"]).unlink()
                return 0

            with (
                patch(
                    "tools.toolchain.commands.tidy.close.tidy_workspace.resolve_workspace",
                    return_value=workspace,
                ),
                patch(
                    "tools.toolchain.commands.tidy.close.TidyRefreshCommand.execute",
                    side_effect=_refresh,
                ),
                patch(
                    "tools.toolchain.commands.tidy.close.VerifyCommand.execute",
                    return_value=0,
                ) as verify_execute,
                patch(
                    "tools.toolchain.commands.tidy.close.TidyStepCommand.execute",
                    side_effect=_step,
                ) as step_execute,
                patch(
                    "tools.toolchain.commands.tidy.close.batch_state.update_state",
                    return_value=root / "batch_state.json",
                ),
                patch(
                    "tools.toolchain.commands.tidy.close.tidy_result_summary.write_tidy_result",
                    side_effect=lambda **kwargs: statuses.append(str(kwargs["status"])),
                ),
            ):
                ret = TidyCloseCommand(ctx).execute(
                    app_name="tracer_core_shell",
                    source_scope="core_family",
                    tidy_build_dir_name="build_tidy_core_family",
                    stabilize=True,
                )

            self.assertEqual(ret, 0)
            self.assertEqual(refresh_calls["count"], 2)
            self.assertEqual(verify_execute.call_count, 2)
            self.assertEqual(step_execute.call_count, 1)
            self.assertEqual(statuses, ["completed"])

    def test_execute_with_stabilize_fails_when_queue_makes_no_progress(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            _write_task_json(tasks_dir / "batch_001" / "task_001.json")

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=tasks_dir,
                batch_state_path=root / "batch_state.json",
                tidy_result_path=root / "tidy_result.json",
            )
            workspace = ResolvedTidyWorkspace(
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
                source_roots=[],
                prebuild_targets=[],
            )

            statuses: list[str] = []

            with (
                patch(
                    "tools.toolchain.commands.tidy.close.tidy_workspace.resolve_workspace",
                    return_value=workspace,
                ),
                patch(
                    "tools.toolchain.commands.tidy.close.TidyRefreshCommand.execute",
                    return_value=0,
                ),
                patch(
                    "tools.toolchain.commands.tidy.close.VerifyCommand.execute",
                    return_value=0,
                ),
                patch(
                    "tools.toolchain.commands.tidy.close.TidyStepCommand.execute",
                    return_value=0,
                ) as step_execute,
                patch(
                    "tools.toolchain.commands.tidy.close.tidy_result_summary.write_tidy_result",
                    side_effect=lambda **kwargs: statuses.append(str(kwargs["status"])),
                ),
            ):
                ret = TidyCloseCommand(ctx).execute(
                    app_name="tracer_core_shell",
                    source_scope="core_family",
                    tidy_build_dir_name="build_tidy_core_family",
                    stabilize=True,
                )

            self.assertEqual(ret, 1)
            self.assertEqual(step_execute.call_count, 1)
            self.assertEqual(statuses, ["stabilize_no_progress"])
