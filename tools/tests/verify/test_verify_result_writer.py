import json
import tempfile
from pathlib import Path
from unittest import TestCase

from tools.toolchain.commands.cmd_quality.verify_internal.verify_result_writer import (
    merge_verify_phase_summary_into_result_json,
    write_build_only_result_json,
)


class TestVerifyResultWriter(TestCase):
    def test_write_build_only_result_json_includes_verify_phase_summary(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            phases = [
                {
                    "name": "build",
                    "category": "verify",
                    "status": "passed",
                    "exit_code": 0,
                },
                {
                    "name": "verify_unit",
                    "category": "verify",
                    "status": "failed",
                    "exit_code": 7,
                },
            ]

            write_build_only_result_json(
                repo_root=repo_root,
                app_name="unknown_app",
                build_dir_name="build_fast",
                success=False,
                exit_code=7,
                duration_seconds=1.25,
                error_message="Unit verification failed.",
                build_only=True,
                verify_phases=phases,
            )

            result_path = repo_root / "out" / "test" / "unknown_app" / "result.json"
            payload = json.loads(result_path.read_text(encoding="utf-8"))

        self.assertEqual(payload["verify_phases"], phases)
        self.assertEqual(
            payload["verify_phase_summary"],
            {"total": 2, "passed": 1, "failed": 1, "skipped": 0},
        )

    def test_merge_verify_phase_summary_into_existing_result_json(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            result_root = repo_root / "out" / "test" / "unknown_app"
            result_root.mkdir(parents=True, exist_ok=True)
            result_path = result_root / "result.json"
            result_path.write_text(
                json.dumps(
                    {
                        "success": True,
                        "exit_code": 0,
                        "total_tests": 3,
                        "total_failed": 0,
                    }
                ),
                encoding="utf-8",
            )

            phases = [
                {
                    "name": "build",
                    "category": "verify",
                    "status": "passed",
                    "exit_code": 0,
                },
                {
                    "name": "artifact.host_blackbox",
                    "category": "artifact",
                    "status": "passed",
                    "exit_code": 0,
                },
            ]
            merge_verify_phase_summary_into_result_json(
                repo_root=repo_root,
                app_name="unknown_app",
                verify_phases=phases,
            )

            payload = json.loads(result_path.read_text(encoding="utf-8"))

        self.assertTrue(payload["success"])
        self.assertEqual(payload["verify_phases"], phases)
        self.assertEqual(
            payload["verify_phase_summary"],
            {"total": 2, "passed": 2, "failed": 0, "skipped": 0},
        )
