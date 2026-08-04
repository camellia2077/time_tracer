import json
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.tests.platform.support.path_assertions import assert_same_path
from tools.toolchain.commands.tidy.queue.task_context import resolve_task_context
from tools.toolchain.commands.tidy.queue.task_log import resolve_task_log_path
from tools.toolchain.commands.tidy.queue.task_model import (
    TaskDiagnostic,
    TaskRecord,
    TaskSummary,
    TaskSummaryEntry,
    task_record_to_dict,
)
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[4]


class TestTidyTaskContext(TestCase):
    def test_resolve_task_log_path_uses_smallest_pending_task(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            (tasks_dir / "clusters" / "cluster_b").mkdir(parents=True)
            (tasks_dir / "clusters" / "cluster_a").mkdir(parents=True)
            first = tasks_dir / "clusters" / "cluster_a" / "task_003.json"
            second = tasks_dir / "clusters" / "cluster_b" / "task_011.json"
            first.write_text("{}\n", encoding="utf-8")
            second.write_text("{}\n", encoding="utf-8")

            resolved = resolve_task_log_path(tasks_dir)
            self.assertEqual(resolved, first)

    def test_resolve_task_context_derives_workspace_from_task_path(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            task_dir = (
                root
                / "out"
                / "tidy"
                / "tracer_core_shell"
                / "build_tidy_core_family"
                / "tasks"
                / "clusters"
                / "cluster_fixture"
            )
            task_dir.mkdir(parents=True)
            source_file = root / "query_api.cpp"
            source_file.write_text("int main() { return 0; }\n", encoding="utf-8")
            record = TaskRecord(
                version=4,
                task_id="003",
                cluster_id="cluster_fixture",
                scan_id="scan_fixture",
                queue_generation=None,
                source_file=str(source_file),
                source_fingerprint=None,
                workspace="build_tidy_core_family",
                source_scope=None,
                checks=("readability-identifier-naming",),
                summary=TaskSummary(
                    diagnostic_count=1,
                    compiler_errors=False,
                    files=(TaskSummaryEntry(name=str(source_file), count=1),),
                    checks=(TaskSummaryEntry(name="readability-identifier-naming", count=1),),
                ),
                diagnostics=(
                    TaskDiagnostic(
                        file=str(source_file),
                        line=1,
                        col=1,
                        severity="warning",
                        check="readability-identifier-naming",
                        message="invalid case style for constant 'value'",
                        raw_lines=(),
                        notes=(),
                    ),
                ),
                snippets=(),
                raw_lines=(),
            )
            task_json_path = task_dir / "task_003.json"
            task_json_path.write_text(
                json.dumps(task_record_to_dict(record), indent=2),
                encoding="utf-8",
            )

            task_ctx = resolve_task_context(Context(REPO_ROOT), task_log_path=str(task_json_path))

            self.assertEqual(task_ctx.app_name, "tracer_core_shell")
            self.assertEqual(task_ctx.tidy_build_dir_name, "build_tidy_core_family")
            self.assertEqual(task_ctx.source_scope, "core_family")
            assert_same_path(task_ctx.tasks_dir, task_dir.parent.parent)
            assert_same_path(task_ctx.task_json_path, task_json_path)

    def test_resolve_task_context_rejects_stale_queue_generation(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            task_dir = (
                root
                / "out"
                / "tidy"
                / "tracer_core_shell"
                / "build_tidy_core_family"
                / "tasks"
                / "clusters"
                / "cluster_fixture"
            )
            task_dir.mkdir(parents=True)
            source_file = root / "query_api.cpp"
            source_file.write_text("int main() { return 0; }\n", encoding="utf-8")
            record = TaskRecord(
                version=4,
                task_id="003",
                cluster_id="cluster_fixture",
                scan_id="scan_fixture",
                queue_generation=1,
                source_file=str(source_file),
                source_fingerprint=None,
                workspace="build_tidy_core_family",
                source_scope=None,
                checks=("readability-identifier-naming",),
                summary=TaskSummary(
                    diagnostic_count=1,
                    compiler_errors=False,
                    files=(TaskSummaryEntry(name=str(source_file), count=1),),
                    checks=(TaskSummaryEntry(name="readability-identifier-naming", count=1),),
                ),
                diagnostics=(
                    TaskDiagnostic(
                        file=str(source_file),
                        line=1,
                        col=1,
                        severity="warning",
                        check="readability-identifier-naming",
                        message="invalid case style for constant 'value'",
                        raw_lines=(),
                        notes=(),
                    ),
                ),
                snippets=(),
                raw_lines=(),
            )
            task_json_path = task_dir / "task_003.json"
            task_json_path.write_text(
                json.dumps(task_record_to_dict(record), indent=2),
                encoding="utf-8",
            )
            (task_dir.parent.parent / "queue_state.json").write_text(
                json.dumps({"queue_generation": 2}, indent=2),
                encoding="utf-8",
            )

            with self.assertRaisesRegex(ValueError, "queue generation is stale"):
                resolve_task_context(Context(REPO_ROOT), task_log_path=str(task_json_path))
