import json
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
from unittest import TestCase

from tools.toolchain.commands.tidy.tidy_result import write_tidy_result
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[3]


class TestTidyResult(TestCase):
    def test_compiler_diagnostic_task_recommends_recheck_first(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks" / "batch_001"
            tasks_dir.mkdir(parents=True)
            result_path = root / "tidy_result.json"
            (tasks_dir / "task_001.json").write_text(
                json.dumps(
                    {
                        "version": 2,
                        "task_id": "001",
                        "batch_id": "batch_001",
                        "queue_batch_id": "batch_001",
                        "source_file": "C:/code/time_tracer/libs/tracer_core/src/demo.cpp",
                        "compiler_errors": True,
                        "checks": ["clang-diagnostic-error"],
                        "diagnostics": [
                            {
                                "line": 7,
                                "col": 8,
                                "severity": "error",
                                "check": "clang-diagnostic-error",
                                "message": "module not found",
                            }
                        ],
                    },
                    indent=2,
                ),
                encoding="utf-8",
            )

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=root / "tasks",
                tasks_done_dir=root / "tasks_done",
                tidy_result_path=result_path,
            )

            write_tidy_result(
                ctx=ctx,
                app_name="tracer_core_shell",
                stage="tidy-batch",
                status="completed",
                exit_code=0,
                build_dir_name="build_tidy_core_family",
                source_scope="core_family",
            )

            payload = json.loads(result_path.read_text(encoding="utf-8"))
            self.assertEqual(payload["recheck_first_candidates"], 1)
            self.assertEqual(
                payload["blocking_files"][0]["recommended_action"],
                "recheck_first",
            )
            self.assertIn("tidy-step", payload["next_action"])
            self.assertIn("--dry-run", payload["next_action"])

    def test_batch_handoff_requires_reresolve_and_reports_current_queue_head(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_root = root / "tasks"
            (tasks_root / "batch_003").mkdir(parents=True)
            result_path = root / "tidy_result.json"
            head_path = tasks_root / "batch_003" / "task_021.json"
            head_path.write_text(
                json.dumps(
                    {
                        "version": 2,
                        "task_id": "021",
                        "batch_id": "batch_003",
                        "queue_batch_id": "batch_003",
                        "source_file": "C:/code/time_tracer/libs/tracer_core/src/next.cpp",
                        "compiler_errors": False,
                        "checks": ["google-runtime-int"],
                        "diagnostics": [
                            {
                                "line": 9,
                                "col": 4,
                                "severity": "warning",
                                "check": "google-runtime-int",
                                "message": "consider replacing 'long long' with 'int64'",
                            }
                        ],
                    },
                    indent=2,
                ),
                encoding="utf-8",
            )

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=tasks_root,
                tasks_done_dir=root / "tasks_done",
                tidy_result_path=result_path,
            )

            write_tidy_result(
                ctx=ctx,
                app_name="tracer_core_shell",
                stage="tidy-batch",
                status="completed",
                exit_code=0,
                build_dir_name="build_tidy_core_family",
                source_scope="core_family",
                historical_batch_id="batch_002",
                historical_task_ids=["011", "012"],
                queue_requires_reresolve=True,
            )

            payload = json.loads(result_path.read_text(encoding="utf-8"))
            self.assertTrue(payload["queue_requires_reresolve"])
            self.assertEqual(payload["queue_head"]["task_id"], "021")
            self.assertEqual(payload["historical_batch"]["batch_id"], "batch_002")
            self.assertTrue(payload["historical_batch"]["historical_identity_stale"])
            self.assertTrue(payload["historical_batch"]["queue_identity_changed_after_refresh"])
            self.assertEqual(payload["replacement_queue_head"]["task_id"], "021")
            self.assertIn("Refresh replaced closed selection", payload["queue_transition_summary"])
            self.assertIn("historical batch/task selection is now stale", payload["next_action"])
            self.assertIn("re-resolve the current smallest pending task", payload["next_action"])
            self.assertIn(str(head_path), payload["next_action"])
