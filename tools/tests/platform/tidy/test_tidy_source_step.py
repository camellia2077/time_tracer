from __future__ import annotations

import json
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.tidy.execution.source_step import TidySourceStepCommand
from tools.toolchain.commands.tidy.execution.step_internal.recheck import TaskRecheckResult
from tools.toolchain.commands.tidy.queue.task_model import (
    TaskDiagnostic,
    TaskRecord,
    TaskSummary,
    TaskSummaryEntry,
    task_record_to_dict,
)
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[4]


def _record(*, task_id: str, cluster_id: str, source_file: str) -> TaskRecord:
    check = "readability-identifier-naming"
    diagnostic = TaskDiagnostic(
        file=source_file,
        line=7,
        col=4,
        severity="warning",
        check=check,
        message="invalid naming",
        raw_lines=(),
        notes=(),
    )
    return TaskRecord(
        version=4,
        task_id=task_id,
        cluster_id=cluster_id,
        scan_id="scan_fixture",
        queue_generation=None,
        source_file=source_file,
        source_fingerprint=None,
        workspace="build_tidy_core_family",
        source_scope="core_family",
        checks=(check,),
        summary=TaskSummary(
            diagnostic_count=1,
            compiler_errors=False,
            files=(TaskSummaryEntry(name=source_file, count=1),),
            checks=(TaskSummaryEntry(name=check, count=1),),
        ),
        diagnostics=(diagnostic,),
        snippets=(),
        raw_lines=(),
    )


class TestTidySourceStepCommand(TestCase):
    def test_processes_all_pending_tasks_for_one_source_and_archives_one_cluster(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            source_file = root / "example.cpp"
            source_file.write_text("int main() { return 0; }\n", encoding="utf-8")
            paths: list[Path] = []
            cluster_id = "cluster_example"
            cluster_dir = tasks_dir / "clusters" / cluster_id
            cluster_dir.mkdir(parents=True)
            for task_id in ("001", "002"):
                task_path = cluster_dir / f"task_{task_id}.json"
                task_path.write_text(
                    json.dumps(
                        task_record_to_dict(
                            _record(task_id=task_id, cluster_id=cluster_id, source_file=str(source_file))
                        )
                    ),
                    encoding="utf-8",
                )
                paths.append(task_path)

            task_ctx = SimpleNamespace(
                app_name="tracer_core_shell",
                tidy_build_dir_name="build_tidy_core_family",
                source_scope="core_family",
                tasks_dir=tasks_dir,
                task_json_path=paths[0],
                current_queue_generation=3,
            )
            workspace = SimpleNamespace(
                build_dir_name="build_tidy_core_family",
                source_scope="core_family",
            )
            ctx = Context(REPO_ROOT)
            command = TidySourceStepCommand(ctx)
            recheck = TaskRecheckResult(
                ok=True,
                exit_code=0,
                log_path=root / "recheck.log",
                remaining_diagnostics=(),
                diagnostics=(),
            )

            with (
                patch(
                    "tools.toolchain.commands.tidy.execution.source_step.resolve_task_context",
                    return_value=task_ctx,
                ),
                patch(
                    "tools.toolchain.commands.tidy.execution.source_step.resolve_workspace",
                    return_value=workspace,
                ),
                patch.object(command, "_cluster_is_stale", return_value=False),
                patch.object(command, "_build", return_value=0) as build,
                patch.object(command, "_run_cluster_recheck", return_value=recheck),
                patch.object(command, "_write_state"),
                patch(
                    "tools.toolchain.commands.tidy.execution.source_step.run_task_auto_fix",
                    side_effect=lambda *args, **kwargs: SimpleNamespace(
                        task_id=Path(kwargs["task_log_path"]).stem.removeprefix("task_"),
                        applied=1,
                        previewed=0,
                        skipped=0,
                        failed=0,
                    ),
                ) as auto_fix,
                patch(
                    "tools.toolchain.commands.tidy.execution.source_step.CleanCommand.execute",
                    return_value=0,
                ) as clean,
            ):
                result = command.execute(task_log_path=str(paths[0]))

            self.assertEqual(result, 0)
            self.assertEqual(auto_fix.call_count, 2)
            build.assert_called_once()
            self.assertEqual(clean.call_count, 1)
            self.assertEqual(clean.call_args.kwargs["cluster_id"], cluster_id)

    def test_dry_run_does_not_build_or_recheck_the_cluster(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            cluster_dir = tasks_dir / "clusters" / "cluster_example"
            cluster_dir.mkdir(parents=True)
            source_file = root / "example.cpp"
            source_file.write_text("int main() { return 0; }\n", encoding="utf-8")
            task_path = cluster_dir / "task_001.json"
            task_path.write_text(
                json.dumps(
                    task_record_to_dict(
                        _record(task_id="001", cluster_id="cluster_example", source_file=str(source_file))
                    )
                ),
                encoding="utf-8",
            )
            task_ctx = SimpleNamespace(
                app_name="tracer_core_shell",
                tidy_build_dir_name="build_tidy_core_family",
                source_scope="core_family",
                tasks_dir=tasks_dir,
                task_json_path=task_path,
                current_queue_generation=None,
            )
            workspace = SimpleNamespace(
                build_dir_name="build_tidy_core_family",
                source_scope="core_family",
            )
            command = TidySourceStepCommand(Context(REPO_ROOT))
            with (
                patch(
                    "tools.toolchain.commands.tidy.execution.source_step.resolve_task_context",
                    return_value=task_ctx,
                ),
                patch(
                    "tools.toolchain.commands.tidy.execution.source_step.resolve_workspace",
                    return_value=workspace,
                ),
                patch.object(command, "_cluster_is_stale", return_value=False),
                patch.object(command, "_build") as build,
                patch.object(command, "_run_cluster_recheck") as recheck,
                patch(
                    "tools.toolchain.commands.tidy.execution.source_step.run_task_auto_fix",
                    return_value=SimpleNamespace(
                        task_id="001", applied=0, previewed=1, skipped=0, failed=0
                    ),
                ),
            ):
                result = command.execute(task_log_path=str(task_path), dry_run=True)

            self.assertEqual(result, 0)
            build.assert_not_called()
            recheck.assert_not_called()
