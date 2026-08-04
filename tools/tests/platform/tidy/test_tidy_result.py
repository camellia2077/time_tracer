import json
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
from unittest import TestCase

from tools.toolchain.commands.tidy.tidy_result import write_tidy_result
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[4]


class TestTidyResult(TestCase):
    def test_cluster_task_recommends_source_cluster_recheck(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            cluster_dir = root / "tasks" / "clusters" / "cluster_demo"
            cluster_dir.mkdir(parents=True)
            (cluster_dir / "task_001.json").write_text(
                json.dumps(
                    {
                        "version": 4,
                        "task_id": "001",
                        "cluster_id": "cluster_demo",
                        "scan_id": "scan_fixture",
                        "source_file": "C:/code/time_tracer/libs/tracer_core/src/demo.cpp",
                        "compiler_errors": True,
                        "checks": ["clang-diagnostic-error"],
                        "diagnostics": [{"line": 7, "col": 8, "severity": "error", "check": "clang-diagnostic-error", "message": "module not found"}],
                    }
                ),
                encoding="utf-8",
            )
            result_path = root / "tidy_result.json"
            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=root / "tasks",
                archive_dir=root / "tasks" / "archive",
                tidy_result_path=result_path,
            )
            write_tidy_result(
                ctx=ctx,
                app_name="tracer_core_shell",
                stage="tidy-source-step",
                status="manual",
                exit_code=2,
                build_dir_name="build_tidy_core_family",
                source_scope="core_family",
            )
            payload = json.loads(result_path.read_text(encoding="utf-8"))
            self.assertEqual(payload["clusters"]["remaining"], 1)
            self.assertEqual(payload["blocking_files"][0]["cluster_id"], "cluster_demo")
            self.assertEqual(payload["blocking_files"][0]["recommended_action"], "recheck_first")
            self.assertIn("tidy-source-step", payload["next_action"])

    def test_final_gate_is_written_without_batch_transition_fields(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            result_path = root / "tidy_result.json"
            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=root / "tasks",
                archive_dir=root / "tasks" / "archive",
                tidy_result_path=result_path,
            )
            write_tidy_result(
                ctx=ctx,
                app_name="tracer_core_shell",
                stage="tidy-close",
                status="completed",
                exit_code=0,
                build_dir_name="build_tidy_core_family",
                final_gate={"final_full_tidy": "passed", "verify": "passed", "queue_empty": "passed"},
            )
            payload = json.loads(result_path.read_text(encoding="utf-8"))
            self.assertEqual(payload["final_gate"]["queue_empty"], "passed")
            self.assertNotIn("historical_batch", payload)
