import json
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.toolchain.commands.tidy.tasking.task_log import (
    list_task_paths,
    load_task_record,
    resolve_task_json_path,
    resolve_task_log_path,
)
from tools.toolchain.commands.tidy.tasking.task_model import TaskDiagnostic, task_record_to_dict

from ..support.tidy_task_model_support import _make_task_record


class TestTidyTaskLogContract(TestCase):
    def test_resolve_task_log_path_prefers_canonical_json(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_001"
            batch_dir.mkdir(parents=True)
            source_file = str(tasks_dir / "demo.cpp")
            record = _make_task_record(
                source_file=source_file,
                diagnostics=(
                    TaskDiagnostic(
                        file=source_file,
                        line=3,
                        col=8,
                        severity="error",
                        check="clang-diagnostic-error",
                        message="module not found",
                        raw_lines=(
                            f"{source_file}:3:8: error: module not found [clang-diagnostic-error]",
                        ),
                        notes=(),
                    ),
                ),
            )
            json_path = batch_dir / "task_001.json"
            json_path.write_text(json.dumps(task_record_to_dict(record), indent=2), encoding="utf-8")
            (batch_dir / "task_001.log").write_text("legacy log\n", encoding="utf-8")

            resolved = resolve_task_log_path(tasks_dir)

            self.assertEqual(resolved, json_path)
            loaded = load_task_record(resolved)
            self.assertEqual(loaded.task_id, "001")
            self.assertEqual(loaded.source_file, source_file)

    def test_resolve_task_json_path_rejects_view_only_artifacts(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_001"
            batch_dir.mkdir(parents=True)
            toon_path = batch_dir / "task_001.toon"
            toon_path.write_text("task:\n", encoding="utf-8")

            with self.assertRaises(FileNotFoundError):
                resolve_task_json_path(tasks_dir, task_log_path=str(toon_path))

    def test_list_task_paths_keeps_legacy_logs_without_json(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_one = tasks_dir / "batch_001"
            batch_two = tasks_dir / "batch_002"
            batch_one.mkdir(parents=True)
            batch_two.mkdir(parents=True)

            source_file = str(tasks_dir / "demo.cpp")
            record = _make_task_record(
                source_file=source_file,
                diagnostics=(
                    TaskDiagnostic(
                        file=source_file,
                        line=3,
                        col=8,
                        severity="error",
                        check="clang-diagnostic-error",
                        message="module not found",
                        raw_lines=(
                            f"{source_file}:3:8: error: module not found [clang-diagnostic-error]",
                        ),
                        notes=(),
                    ),
                ),
            )
            (batch_one / "task_001.json").write_text(
                json.dumps(task_record_to_dict(record), indent=2),
                encoding="utf-8",
            )
            (batch_one / "task_001.log").write_text("legacy duplicate\n", encoding="utf-8")
            (batch_two / "task_002.log").write_text(
                "\n".join(
                    [
                        f"File: {source_file}",
                        f"{source_file}:9:4: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                    ]
                )
                + "\n",
                encoding="utf-8",
            )

            task_paths = list_task_paths(tasks_dir)

            self.assertEqual(
                task_paths,
                [
                    batch_one / "task_001.json",
                    batch_two / "task_002.log",
                ],
            )
