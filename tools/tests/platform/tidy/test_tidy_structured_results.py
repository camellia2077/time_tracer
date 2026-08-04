from __future__ import annotations

import json
import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.tests.platform.support.tidy_task_auto_fix_support import AutoFixFixtureBuilder
from tools.toolchain.commands.tidy.scan.structured_results import collect_structured_results
from tools.toolchain.commands.clang.tidy.invocation import run_wrapper


class TestTidyStructuredResults(TestCase):
    def test_wrapper_writes_structured_diagnostics(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            output = root / "structured" / "check_001.json"
            source_file = root / "example.cpp"
            source_file.write_text("void Run() {\n  const auto payload = 1;\n}\n", encoding="utf-8")

            code = (
                "print(r\"{}:2:14: warning: invalid case style for constant 'payload' "
                "[readability-identifier-naming]\")"
            ).format(source_file)
            result = run_wrapper(
                output=output,
                source_file=source_file,
                command=[sys.executable, "-c", code],
            )

            self.assertEqual(result, 0)
            payload = json.loads(output.read_text(encoding="utf-8"))
            self.assertEqual(payload["version"], 1)
            self.assertEqual(payload["source_file"], str(source_file))
            self.assertEqual(payload["diagnostics"][0]["check"], "readability-identifier-naming")

    def test_collector_writes_current_source_cluster_task_json(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            fixture = AutoFixFixtureBuilder(root)
            source_file = fixture.write_source(
                "libs/tracer_core/src/application/example.cpp",
                ["void Run() {", "  const auto payload = 1;", "}"],
            )
            results_dir = fixture.build_tidy_dir / "structured_tidy_results"
            results_dir.mkdir(parents=True)
            archive_dir = fixture.tasks_dir / "archive" / "cluster_previous"
            archive_dir.mkdir(parents=True)
            archived_marker = archive_dir / "task_001.json"
            archived_marker.write_text("{\"archived\": true}\n", encoding="utf-8")
            (results_dir / "check_001.json").write_text(
                json.dumps(
                    {
                        "version": 1,
                        "source_file": str(source_file),
                        "exit_code": 0,
                        "diagnostics": [
                            {
                                "file": str(source_file),
                                "line": 2,
                                "col": 14,
                                "severity": "warning",
                                "check": "readability-identifier-naming",
                                "message": "invalid case style for constant 'payload'",
                                "lines": [
                                    f"{source_file}:2:14: warning: invalid case style for constant 'payload' "
                                    "[readability-identifier-naming]",
                                    "    | kPayload",
                                ],
                            }
                        ],
                    }
                ),
                encoding="utf-8",
            )

            stats, _elapsed = collect_structured_results(
                fixture.context(),
                results_dir,
                fixture.tasks_dir,
                task_view="json",
                workspace_name=fixture.build_dir_name,
                source_scope=fixture.source_scope,
            )

            cluster_dirs = list((fixture.tasks_dir / "clusters").iterdir())
            task_json = next((cluster_dirs[0]).glob("task_001.json"))
            task = json.loads(task_json.read_text(encoding="utf-8"))
            self.assertEqual(stats["input"], "structured")
            self.assertEqual(stats["tasks"], 1)
            self.assertTrue(archived_marker.exists())
            self.assertEqual(task["source_file"], str(source_file))
            self.assertEqual(task["diagnostics"][0]["check"], "readability-identifier-naming")
