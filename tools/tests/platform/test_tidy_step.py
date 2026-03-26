import json
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.tidy.task_auto_fix import AutoFixAction, TaskAutoFixResult
from tools.toolchain.commands.tidy.step import TaskRecheckResult, TidyStepCommand
from tools.toolchain.commands.tidy.task_model import (
    TaskDiagnostic,
    TaskRecord,
    TaskSummary,
    TaskSummaryEntry,
    task_record_to_dict,
)
from tools.toolchain.commands.tidy.workspace import ResolvedTidyWorkspace
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[3]


def _make_task_record(
    *,
    task_id: str = "011",
    batch_id: str = "batch_002",
    source_file: str,
    check: str = "readability-identifier-naming",
) -> TaskRecord:
    diagnostics = (
        TaskDiagnostic(
            file=source_file,
            line=7,
            col=4,
            severity="warning",
            check=check,
            message="invalid case style for variable 'payload'",
            raw_lines=(
                f"{source_file}:7:4: warning: invalid case style for variable 'payload' [{check}]",
            ),
            notes=(),
        ),
    )
    summary = TaskSummary(
        diagnostic_count=1,
        compiler_errors=False,
        files=(TaskSummaryEntry(name=source_file, count=1),),
        checks=(TaskSummaryEntry(name=check, count=1),),
    )
    return TaskRecord(
        version=2,
        task_id=task_id,
        batch_id=batch_id,
        source_file=source_file,
        workspace="build_tidy_core_family",
        source_scope="core_family",
        checks=(check,),
        summary=summary,
        diagnostics=diagnostics,
        snippets=(),
        raw_lines=diagnostics[0].raw_lines,
    )


class TestTidyStepCommand(TestCase):
    def test_execute_continues_after_stale_rename_failures(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            tasks_done_dir = root / "tasks_done"
            automation_dir = root / "automation"
            batch_dir = tasks_dir / "batch_002"
            batch_dir.mkdir(parents=True)
            automation_dir.mkdir(parents=True)

            source_file = str(root / "example.cpp")
            task_path = batch_dir / "task_012.toon"
            task_path.write_text("task artifact\n", encoding="utf-8")
            record = _make_task_record(task_id="012", source_file=source_file)

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                root=root,
                tasks_dir=tasks_dir,
                tasks_done_dir=tasks_done_dir,
                automation_dir=automation_dir,
                batch_state_path=root / "batch_state.json",
                tidy_result_path=root / "tidy_result.json",
            )

            command = TidyStepCommand(ctx)
            workspace = ResolvedTidyWorkspace(
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
                source_roots=[],
                prebuild_targets=[],
            )
            recheck_result = TaskRecheckResult(
                ok=True,
                exit_code=0,
                log_path=automation_dir / "batch_002_task_012_recheck.log",
                remaining_diagnostics=(),
            )
            fix_result = TaskAutoFixResult(
                app_name="tracer_core_shell",
                task_id="012",
                batch_id="batch_002",
                task_log=str(task_path),
                source_file=source_file,
                mode="apply",
                workspace="build_tidy_core_family",
                source_scope="core_family",
                skipped=1,
                failed=1,
                actions=[
                    AutoFixAction(
                        action_id="rename:001",
                        kind="rename",
                        file_path=source_file,
                        line=7,
                        col=4,
                        check="readability-identifier-naming",
                        status="failed",
                        reason=(
                            "{'code': -32001, 'message': "
                            "'Cannot rename symbol: there is no symbol at the given location'}"
                        ),
                    )
                ],
                json_path=str(automation_dir / "batch_002_task_012_step.json"),
                markdown_path=str(automation_dir / "batch_002_task_012_step.md"),
            )

            with (
                patch("tools.toolchain.commands.tidy.step.resolve_workspace", return_value=workspace),
                patch("tools.toolchain.commands.tidy.step.resolve_task_log_path", return_value=task_path),
                patch("tools.toolchain.commands.tidy.step.parse_task_log", return_value=record),
                patch("tools.toolchain.commands.tidy.step.run_task_auto_fix", return_value=fix_result),
                patch("tools.toolchain.commands.tidy.step.BuildCommand.build", return_value=0),
                patch.object(TidyStepCommand, "_run_task_recheck", return_value=recheck_result),
            ):
                ret = command.execute(
                    app_name="tracer_core_shell",
                    batch_id="002",
                    task_id="012",
                    tidy_build_dir_name="build_tidy_core_family",
                    source_scope="core_family",
                )

            self.assertEqual(ret, 0)
            self.assertFalse(task_path.exists())
            self.assertTrue((tasks_done_dir / "batch_002" / "task_012.toon").exists())

    def test_execute_continues_after_unsupported_kind_rename_failures(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            tasks_done_dir = root / "tasks_done"
            automation_dir = root / "automation"
            batch_dir = tasks_dir / "batch_002"
            batch_dir.mkdir(parents=True)
            automation_dir.mkdir(parents=True)

            source_file = str(root / "example.cpp")
            task_path = batch_dir / "task_013.toon"
            task_path.write_text("task artifact\n", encoding="utf-8")
            record = _make_task_record(task_id="013", source_file=source_file)

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                root=root,
                tasks_dir=tasks_dir,
                tasks_done_dir=tasks_done_dir,
                automation_dir=automation_dir,
                batch_state_path=root / "batch_state.json",
                tidy_result_path=root / "tidy_result.json",
            )

            command = TidyStepCommand(ctx)
            workspace = ResolvedTidyWorkspace(
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
                source_roots=[],
                prebuild_targets=[],
            )
            recheck_result = TaskRecheckResult(
                ok=True,
                exit_code=0,
                log_path=automation_dir / "batch_002_task_013_recheck.log",
                remaining_diagnostics=(),
            )
            fix_result = TaskAutoFixResult(
                app_name="tracer_core_shell",
                task_id="013",
                batch_id="batch_002",
                task_log=str(task_path),
                source_file=source_file,
                mode="apply",
                workspace="build_tidy_core_family",
                source_scope="core_family",
                skipped=1,
                failed=1,
                actions=[
                    AutoFixAction(
                        action_id="rename:001",
                        kind="rename",
                        file_path=source_file,
                        line=7,
                        col=4,
                        check="readability-identifier-naming",
                        status="failed",
                        reason=(
                            "{'code': -32001, 'message': "
                            "'Cannot rename symbol: symbol is not a supported kind "
                            "(e.g. namespace, macro)'}"
                        ),
                    )
                ],
                json_path=str(automation_dir / "batch_002_task_013_step.json"),
                markdown_path=str(automation_dir / "batch_002_task_013_step.md"),
            )

            with (
                patch("tools.toolchain.commands.tidy.step.resolve_workspace", return_value=workspace),
                patch("tools.toolchain.commands.tidy.step.resolve_task_log_path", return_value=task_path),
                patch("tools.toolchain.commands.tidy.step.parse_task_log", return_value=record),
                patch("tools.toolchain.commands.tidy.step.run_task_auto_fix", return_value=fix_result),
                patch("tools.toolchain.commands.tidy.step.BuildCommand.build", return_value=0),
                patch.object(TidyStepCommand, "_run_task_recheck", return_value=recheck_result),
            ):
                ret = command.execute(
                    app_name="tracer_core_shell",
                    batch_id="002",
                    task_id="013",
                    tidy_build_dir_name="build_tidy_core_family",
                    source_scope="core_family",
                )

            self.assertEqual(ret, 0)
            self.assertFalse(task_path.exists())
            self.assertTrue((tasks_done_dir / "batch_002" / "task_013.toon").exists())

    def test_execute_archives_current_task_after_successful_recheck(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            tasks_done_dir = root / "tasks_done"
            automation_dir = root / "automation"
            batch_dir = tasks_dir / "batch_002"
            batch_dir.mkdir(parents=True)
            automation_dir.mkdir(parents=True)

            source_file = str(root / "example.cpp")
            task_path = batch_dir / "task_011.toon"
            task_path.write_text("task artifact\n", encoding="utf-8")
            sibling_record = _make_task_record(task_id="012", source_file=source_file)
            sibling_json_path = batch_dir / "task_012.json"
            sibling_json_path.write_text(
                json.dumps(task_record_to_dict(sibling_record), indent=2),
                encoding="utf-8",
            )
            record = _make_task_record(source_file=source_file)

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                root=root,
                tasks_dir=tasks_dir,
                tasks_done_dir=tasks_done_dir,
                automation_dir=automation_dir,
                batch_state_path=root / "batch_state.json",
                tidy_result_path=root / "tidy_result.json",
            )

            command = TidyStepCommand(ctx)
            workspace = ResolvedTidyWorkspace(
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
                source_roots=[],
                prebuild_targets=[],
            )
            recheck_result = TaskRecheckResult(
                ok=True,
                exit_code=0,
                log_path=automation_dir / "batch_002_task_011_recheck.log",
                remaining_diagnostics=(),
            )
            fix_result = TaskAutoFixResult(
                app_name="tracer_core_shell",
                task_id="011",
                batch_id="batch_002",
                task_log=str(task_path),
                source_file=source_file,
                mode="apply",
                workspace="build_tidy_core_family",
                source_scope="core_family",
                skipped=1,
                json_path=str(automation_dir / "batch_002_task_011_step.json"),
                markdown_path=str(automation_dir / "batch_002_task_011_step.md"),
            )

            with (
                patch("tools.toolchain.commands.tidy.step.resolve_workspace", return_value=workspace),
                patch("tools.toolchain.commands.tidy.step.resolve_task_log_path", return_value=task_path),
                patch("tools.toolchain.commands.tidy.step.parse_task_log", return_value=record),
                patch("tools.toolchain.commands.tidy.step.run_task_auto_fix", return_value=fix_result),
                patch("tools.toolchain.commands.tidy.step.BuildCommand.build", return_value=0),
                patch.object(TidyStepCommand, "_run_task_recheck", return_value=recheck_result),
            ):
                ret = command.execute(
                    app_name="tracer_core_shell",
                    batch_id="002",
                    task_id="011",
                    tidy_build_dir_name="build_tidy_core_family",
                    source_scope="core_family",
                )

            self.assertEqual(ret, 0)
            self.assertFalse(task_path.exists())
            self.assertTrue((tasks_done_dir / "batch_002" / "task_011.toon").exists())
            self.assertTrue(sibling_json_path.exists())

            state_payload = json.loads(
                (automation_dir / "tidy_step_last.json").read_text(encoding="utf-8")
            )
            self.assertTrue(state_payload["task_archived"])
            self.assertEqual(
                state_payload["recheck_log"],
                str(recheck_result.log_path),
            )
            self.assertEqual(state_payload["handoff_batch_id"], "batch_002")
            self.assertTrue(state_payload["queue_requires_reresolve_after_batch"])
            self.assertIn("If you continue with batch handoff", state_payload["next_action"])
            self.assertIn("re-resolve the current smallest pending task", state_payload["next_action"])

            batch_state_payload = json.loads((root / "batch_state.json").read_text(encoding="utf-8"))
            self.assertEqual(batch_state_payload["batch_id"], "batch_002")
            self.assertEqual(batch_state_payload["queue_batch_id"], "batch_002")
            self.assertTrue(batch_state_payload["last_tidy_step_ok"])
            self.assertEqual(batch_state_payload["next_queue_head"]["task_id"], "012")
            self.assertIn("Closed batch_002/task_011.", batch_state_payload["queue_transition_summary"])
            self.assertIn("task_012", batch_state_payload["next_action"])

            result_payload = json.loads((root / "tidy_result.json").read_text(encoding="utf-8"))
            self.assertEqual(result_payload["stage"], "tidy-step")
            self.assertEqual(result_payload["status"], "task_archived")
            self.assertEqual(result_payload["queue_head"]["task_id"], "012")
            self.assertEqual(result_payload["tasks"]["remaining"], 1)
            self.assertIn("task_012", result_payload["next_action"])

    def test_execute_keeps_task_when_recheck_finds_remaining_diagnostic(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            tasks_done_dir = root / "tasks_done"
            automation_dir = root / "automation"
            batch_dir = tasks_dir / "batch_002"
            batch_dir.mkdir(parents=True)
            automation_dir.mkdir(parents=True)

            source_file = str(root / "example.cpp")
            task_path = batch_dir / "task_011.toon"
            task_path.write_text("task artifact\n", encoding="utf-8")
            record = _make_task_record(source_file=source_file)

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                root=root,
                tasks_dir=tasks_dir,
                tasks_done_dir=tasks_done_dir,
                automation_dir=automation_dir,
                batch_state_path=root / "batch_state.json",
                tidy_result_path=root / "tidy_result.json",
            )

            command = TidyStepCommand(ctx)
            workspace = ResolvedTidyWorkspace(
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
                source_roots=[],
                prebuild_targets=[],
            )
            recheck_result = TaskRecheckResult(
                ok=False,
                exit_code=1,
                log_path=automation_dir / "batch_002_task_011_recheck.log",
                remaining_diagnostics=(
                    {
                        "file": source_file,
                        "line": 7,
                        "col": 4,
                        "check": "readability-identifier-naming",
                        "message": "invalid case style for variable 'payload'",
                    },
                ),
            )
            fix_result = TaskAutoFixResult(
                app_name="tracer_core_shell",
                task_id="011",
                batch_id="batch_002",
                task_log=str(task_path),
                source_file=source_file,
                mode="apply",
                workspace="build_tidy_core_family",
                source_scope="core_family",
                skipped=1,
                json_path=str(automation_dir / "batch_002_task_011_step.json"),
                markdown_path=str(automation_dir / "batch_002_task_011_step.md"),
            )

            with (
                patch("tools.toolchain.commands.tidy.step.resolve_workspace", return_value=workspace),
                patch("tools.toolchain.commands.tidy.step.resolve_task_log_path", return_value=task_path),
                patch("tools.toolchain.commands.tidy.step.parse_task_log", return_value=record),
                patch("tools.toolchain.commands.tidy.step.run_task_auto_fix", return_value=fix_result),
                patch("tools.toolchain.commands.tidy.step.BuildCommand.build", return_value=0),
                patch.object(TidyStepCommand, "_run_task_recheck", return_value=recheck_result),
            ):
                ret = command.execute(
                    app_name="tracer_core_shell",
                    batch_id="002",
                    task_id="011",
                    tidy_build_dir_name="build_tidy_core_family",
                    source_scope="core_family",
                )

            self.assertEqual(ret, 1)
            self.assertTrue(task_path.exists())
            self.assertFalse(tasks_done_dir.exists())

    def test_execute_stops_after_single_task_archive_and_requires_reresolve(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            tasks_done_dir = root / "tasks_done"
            automation_dir = root / "automation"
            batch_dir = tasks_dir / "batch_002"
            batch_dir.mkdir(parents=True)
            automation_dir.mkdir(parents=True)

            source_file = str(root / "example.cpp")
            task_path = batch_dir / "task_011.toon"
            task_path.write_text("task artifact\n", encoding="utf-8")
            record = _make_task_record(source_file=source_file)

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                root=root,
                tasks_dir=tasks_dir,
                tasks_done_dir=tasks_done_dir,
                automation_dir=automation_dir,
                batch_state_path=root / "batch_state.json",
                tidy_result_path=root / "tidy_result.json",
            )

            command = TidyStepCommand(ctx)
            workspace = ResolvedTidyWorkspace(
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
                source_roots=[],
                prebuild_targets=[],
            )
            recheck_result = TaskRecheckResult(
                ok=True,
                exit_code=0,
                log_path=automation_dir / "batch_002_task_011_recheck.log",
                remaining_diagnostics=(),
            )
            fix_result = TaskAutoFixResult(
                app_name="tracer_core_shell",
                task_id="011",
                batch_id="batch_002",
                task_log=str(task_path),
                source_file=source_file,
                mode="apply",
                workspace="build_tidy_core_family",
                source_scope="core_family",
                skipped=1,
                json_path=str(automation_dir / "batch_002_task_011_step.json"),
                markdown_path=str(automation_dir / "batch_002_task_011_step.md"),
            )

            with (
                patch("tools.toolchain.commands.tidy.step.resolve_workspace", return_value=workspace),
                patch("tools.toolchain.commands.tidy.step.resolve_task_log_path", return_value=task_path),
                patch("tools.toolchain.commands.tidy.step.parse_task_log", return_value=record),
                patch("tools.toolchain.commands.tidy.step.run_task_auto_fix", return_value=fix_result),
                patch("tools.toolchain.commands.tidy.step.BuildCommand.build", return_value=0),
                patch.object(TidyStepCommand, "_run_task_recheck", return_value=recheck_result),
            ):
                ret = command.execute(
                    app_name="tracer_core_shell",
                    batch_id="002",
                    task_id="011",
                    tidy_build_dir_name="build_tidy_core_family",
                    source_scope="core_family",
                )

            self.assertEqual(ret, 0)
            self.assertFalse(task_path.exists())
            state_payload = json.loads(
                (automation_dir / "tidy_step_last.json").read_text(encoding="utf-8")
            )
            self.assertTrue(state_payload["single_task_batch_closed"])
            self.assertTrue(state_payload["historical_selection_stale_after_close"])
            self.assertTrue(state_payload["queue_requires_reresolve_after_close"])
            self.assertIsNone(state_payload["next_queue_head_after_close"])
            self.assertIn("Stop here and re-resolve the current queue", state_payload["next_action"])
            self.assertIn("do not keep using batch_002/task_011", state_payload["next_action"])

            batch_state_payload = json.loads((root / "batch_state.json").read_text(encoding="utf-8"))
            self.assertIsNone(batch_state_payload["batch_id"])
            self.assertIsNone(batch_state_payload["queue_batch_id"])
            self.assertIsNone(batch_state_payload["next_queue_head"])
            self.assertIn("Closed batch_002/task_011.", batch_state_payload["queue_transition_summary"])

            result_payload = json.loads((root / "tidy_result.json").read_text(encoding="utf-8"))
            self.assertEqual(result_payload["stage"], "tidy-step")
            self.assertEqual(result_payload["status"], "task_archived")
            self.assertIsNone(result_payload["queue_head"])
            self.assertEqual(result_payload["tasks"]["remaining"], 0)
