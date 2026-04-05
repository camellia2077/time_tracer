import json
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.toolchain.commands.tidy.refresh_internal.refresh_mapper import collect_batch_files
from tools.toolchain.commands.tidy.tasking.task_model import TaskDiagnostic, TaskSnippet, render_toon, task_record_to_dict
from tools.toolchain.services.rename_planner import collect_rename_candidates

from ..support.tidy_task_model_support import _make_task_record


class TestTidyTaskCollectors(TestCase):
    def test_collect_batch_files_reads_canonical_json(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_001"
            batch_dir.mkdir(parents=True)
            source_file = str(tasks_dir / "primary.cpp")
            header_file = str(tasks_dir / "detail.hpp")
            record = _make_task_record(
                source_file=source_file,
                diagnostics=(
                    TaskDiagnostic(
                        file=header_file,
                        line=7,
                        col=4,
                        severity="warning",
                        check="readability-identifier-naming",
                        message="invalid case style for variable 'payload'",
                        raw_lines=(
                            f"{header_file}:7:4: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                            "    | kPayload",
                        ),
                        notes=(),
                    ),
                ),
                checks=("readability-identifier-naming",),
            )
            (batch_dir / "task_001.json").write_text(
                json.dumps(task_record_to_dict(record), indent=2),
                encoding="utf-8",
            )

            touched = collect_batch_files(batch_dir)

            self.assertEqual(
                [str(path) for path in touched],
                sorted([header_file, source_file], key=lambda item: item.lower()),
            )

    def test_collect_rename_candidates_reads_json_records(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_001"
            batch_dir.mkdir(parents=True)
            source_file = str(tasks_dir / "example.cpp")
            record = _make_task_record(
                source_file=source_file,
                diagnostics=(
                    TaskDiagnostic(
                        file=source_file,
                        line=3,
                        col=9,
                        severity="warning",
                        check="readability-identifier-naming",
                        message="invalid case style for variable 'payload'",
                        raw_lines=(
                            f"{source_file}:3:9: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                            "    | kPayload",
                        ),
                        notes=(),
                    ),
                ),
                checks=("readability-identifier-naming",),
            )
            (batch_dir / "task_001.json").write_text(
                json.dumps(task_record_to_dict(record), indent=2),
                encoding="utf-8",
            )

            candidates = collect_rename_candidates(
                tasks_dir=tasks_dir,
                check_name="readability-identifier-naming",
                allowed_kinds=["variable"],
            )

            self.assertEqual(len(candidates), 1)
            self.assertEqual(candidates[0]["new_name"], "kPayload")

    def test_collect_rename_candidates_reads_toon_records(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_001"
            batch_dir.mkdir(parents=True)
            source_file = str(tasks_dir / "example.cpp")
            record = _make_task_record(
                source_file=source_file,
                diagnostics=(
                    TaskDiagnostic(
                        file=source_file,
                        line=3,
                        col=9,
                        severity="warning",
                        check="readability-identifier-naming",
                        message="invalid case style for variable 'payload'",
                        raw_lines=(
                            f"{source_file}:3:9: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                            "    | kPayload",
                        ),
                        notes=(),
                    ),
                ),
                snippets=(
                    TaskSnippet(
                        diagnostic_index=1,
                        source_line=None,
                        code="",
                        caret="kPayload",
                        notes=(),
                    ),
                ),
                checks=("readability-identifier-naming",),
            )
            (batch_dir / "task_001.toon").write_text(
                render_toon(record),
                encoding="utf-8",
            )

            candidates = collect_rename_candidates(
                tasks_dir=tasks_dir,
                check_name="readability-identifier-naming",
                allowed_kinds=["variable"],
            )

            self.assertEqual(len(candidates), 1)
            self.assertEqual(candidates[0]["new_name"], "kPayload")
