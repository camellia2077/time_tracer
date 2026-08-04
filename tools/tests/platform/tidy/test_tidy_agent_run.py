from __future__ import annotations

import json
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.tidy.execution.agent_run import TidyAgentRunCommand
from tools.toolchain.commands.tidy.queue.source_cluster import collect_source_clusters
from tools.toolchain.commands.tidy.queue.task_log import task_view_paths
from tools.toolchain.commands.tidy.queue.task_model import (
    TaskDiagnostic,
    TaskRecord,
    TaskSummary,
    TaskSummaryEntry,
    task_record_to_dict,
)
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[4]


def _write_task(tasks_dir: Path, *, task_id: str, source_file: Path) -> Path:
    cluster_dir = tasks_dir / "clusters" / "cluster_fixture"
    cluster_dir.mkdir(parents=True, exist_ok=True)
    check = "readability-identifier-naming"
    diagnostic = TaskDiagnostic(
        file=str(source_file),
        line=1,
        col=1,
        severity="warning",
        check=check,
        message="invalid naming",
        raw_lines=(),
        notes=(),
    )
    record = TaskRecord(
        version=4,
        task_id=task_id,
        cluster_id="cluster_fixture",
        scan_id="scan_fixture",
        queue_generation=None,
        source_file=str(source_file),
        source_fingerprint=None,
        workspace="build_tidy_core_family",
        source_scope="core_family",
        checks=(check,),
        summary=TaskSummary(
            diagnostic_count=1,
            compiler_errors=False,
            files=(TaskSummaryEntry(name=str(source_file), count=1),),
            checks=(TaskSummaryEntry(name=check, count=1),),
        ),
        diagnostics=(diagnostic,),
        snippets=(),
        raw_lines=(),
    )
    task_path = cluster_dir / f"task_{task_id}.json"
    task_path.write_text(json.dumps(task_record_to_dict(record)), encoding="utf-8")
    return task_path


class TestTidyAgentRunCommand(TestCase):
    def test_dry_run_previews_one_cluster_and_leaves_queue_intact(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            task_path = _write_task(tasks_dir, task_id="001", source_file=root / "only.cpp")
            automation_dir = root / "automation"
            automation_dir.mkdir()
            layout = SimpleNamespace(tasks_dir=tasks_dir, automation_dir=automation_dir)
            workspace = SimpleNamespace(source_scope="core_family", build_dir_name="build_tidy_core_family")
            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: layout

            with (
                patch(
                    "tools.toolchain.commands.tidy.execution.agent_run.resolve_workspace",
                    return_value=workspace,
                ),
                patch(
                    "tools.toolchain.commands.tidy.execution.agent_run.TidySourceStepCommand.execute",
                    return_value=0,
                ) as step,
            ):
                result = TidyAgentRunCommand(ctx).execute(
                    app_name="tracer_core_shell",
                    source_scope="core_family",
                    tidy_build_dir_name="build_tidy_core_family",
                    max_clusters=3,
                    max_tasks=10,
                    max_minutes=1,
                    dry_run=True,
                )

            state = json.loads((automation_dir / "agent_run_state.json").read_text(encoding="utf-8"))
            self.assertEqual(result, 0)
            self.assertEqual(state["status"], "previewed")
            self.assertEqual(state["reason"], "dry_run")
            self.assertEqual(state["processed_clusters"], 0)
            self.assertTrue(state["dry_run"])
            self.assertTrue(task_path.exists())
            self.assertTrue(step.call_args.kwargs["dry_run"])

    def test_re_resolves_queue_after_each_cluster_and_stops_on_manual_cluster(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            first_source = root / "first.cpp"
            second_source = root / "second.cpp"
            first_source.write_text("int first;\n", encoding="utf-8")
            second_source.write_text("int second;\n", encoding="utf-8")
            _write_task(tasks_dir, task_id="001", source_file=first_source)
            _write_task(tasks_dir, task_id="002", source_file=second_source)
            automation_dir = root / "automation"
            automation_dir.mkdir()
            layout = SimpleNamespace(
                tasks_dir=tasks_dir,
                automation_dir=automation_dir,
                build_dir_name="build_tidy_core_family",
            )
            workspace = SimpleNamespace(
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
            )
            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: layout
            calls = 0

            def fake_step(*args, **kwargs):
                nonlocal calls
                calls += 1
                cluster = collect_source_clusters(tasks_dir)[0]
                if calls == 1:
                    for task_path in cluster.task_paths:
                        for artifact in task_view_paths(task_path):
                            artifact.unlink()
                    return 0
                return 2

            with (
                patch(
                    "tools.toolchain.commands.tidy.execution.agent_run.resolve_workspace",
                    return_value=workspace,
                ),
                patch(
                    "tools.toolchain.commands.tidy.execution.agent_run.TidySourceStepCommand.execute",
                    side_effect=fake_step,
                ),
            ):
                result = TidyAgentRunCommand(ctx).execute(
                    app_name="tracer_core_shell",
                    source_scope="core_family",
                    tidy_build_dir_name="build_tidy_core_family",
                    max_clusters=3,
                    max_tasks=10,
                    max_minutes=1,
                )

            state = json.loads((automation_dir / "agent_run_state.json").read_text(encoding="utf-8"))
            self.assertEqual(result, 2)
            self.assertEqual(calls, 2)
            self.assertEqual(state["status"], "blocked")
            self.assertEqual(state["processed_clusters"], 1)
            self.assertEqual(state["processed_tasks"], 1)
            self.assertEqual(state["remaining_tasks"], 1)
            self.assertEqual(state["remaining_clusters"], 1)
            self.assertEqual(state["queue_head"]["task_id"], "002")
            self.assertIn("manual", state["next_action"])

    def test_empty_queue_is_a_normal_pause_with_tidy_close_next_action(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            task_path = _write_task(
                tasks_dir,
                task_id="001",
                source_file=root / "only.cpp",
            )
            (root / "only.cpp").write_text("int only;\n", encoding="utf-8")
            automation_dir = root / "automation"
            automation_dir.mkdir()
            layout = SimpleNamespace(tasks_dir=tasks_dir, automation_dir=automation_dir)
            workspace = SimpleNamespace(source_scope="core_family", build_dir_name="build_tidy_core_family")
            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: layout

            def fake_step(*args, **kwargs):
                for artifact in task_view_paths(task_path):
                    artifact.unlink()
                return 0

            with (
                patch(
                    "tools.toolchain.commands.tidy.execution.agent_run.resolve_workspace",
                    return_value=workspace,
                ),
                patch(
                    "tools.toolchain.commands.tidy.execution.agent_run.TidySourceStepCommand.execute",
                    side_effect=fake_step,
                ),
            ):
                result = TidyAgentRunCommand(ctx).execute(
                    app_name="tracer_core_shell",
                    source_scope="core_family",
                    tidy_build_dir_name="build_tidy_core_family",
                    max_clusters=1,
                    max_tasks=1,
                    max_minutes=1,
                )

            state = json.loads((automation_dir / "agent_run_state.json").read_text(encoding="utf-8"))
            self.assertEqual(result, 0)
            self.assertEqual(state["reason"], "queue_empty_requires_tidy_close")
            self.assertIn("tidy-close", state["next_action"])
