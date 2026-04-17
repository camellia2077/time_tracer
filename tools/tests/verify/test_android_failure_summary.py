import json
import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.commands.shared.android_failure_summary import (  # noqa: E402
    build_android_failure_summary,
    render_android_failure_summary,
)


class TestAndroidFailureSummary(TestCase):
    def test_build_android_failure_summary_extracts_gradle_task_and_test_context(self) -> None:
        with TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            build_log = temp_root / "build.log"
            aggregated_log = temp_root / "output.log"
            full_log = temp_root / "output_full.log"
            result_json = temp_root / "result.json"

            build_log.write_text(
                "\n".join(
                    [
                        "> Task :app:testDebugUnitTest FAILED",
                        "com.example.tracer.data.UserPreferencesRepositoryTest > setReportPiePalettePreset_persistsSelection FAILED",
                        "Caused by: java.io.IOException at UserPreferencesRepositoryTest.kt:68",
                        "50 tests completed, 4 failed",
                    ]
                ),
                encoding="utf-8",
            )
            aggregated_log.write_text(
                "Execution failed for task ':app:testDebugUnitTest'.",
                encoding="utf-8",
            )
            full_log.write_text("BUILD FAILED in 7s", encoding="utf-8")
            result_json.write_text(
                json.dumps(
                    {
                        "verify_phases": [
                            {
                                "name": "build",
                                "category": "verify",
                                "status": "failed",
                                "exit_code": 1,
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )

            summary = build_android_failure_summary(
                profile="android_ci",
                build_log_path=build_log,
                aggregated_log_path=aggregated_log,
                full_log_path=full_log,
                result_json_path=result_json,
            )
            rendered = render_android_failure_summary(summary)

        self.assertEqual(summary.profile, "android_ci")
        self.assertEqual(summary.stage, "build")
        self.assertEqual(summary.gradle_task, ":app:testDebugUnitTest")
        self.assertEqual(
            summary.primary_error,
            "Caused by: java.io.IOException at UserPreferencesRepositoryTest.kt:68",
        )
        self.assertIn(
            "com.example.tracer.data.UserPreferencesRepositoryTest > setReportPiePalettePreset_persistsSelection",
            summary.suspected_tests,
        )
        self.assertIn("UserPreferencesRepositoryTest.kt:68", summary.source_locations)
        self.assertIn("=== ANDROID CI FAILURE SUMMARY ===", rendered)
        self.assertIn("gradle_task: :app:testDebugUnitTest", rendered)
        self.assertIn("source_locations:", rendered)

    def test_build_android_failure_summary_handles_missing_files(self) -> None:
        with TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            summary = build_android_failure_summary(
                profile="android_style",
                build_log_path=temp_root / "missing-build.log",
                aggregated_log_path=temp_root / "missing-output.log",
                full_log_path=temp_root / "missing-output-full.log",
                result_json_path=temp_root / "missing-result.json",
            )

        self.assertEqual(summary.profile, "android_style")
        self.assertIsNone(summary.stage)
        self.assertIsNone(summary.gradle_task)
        self.assertIsNone(summary.primary_error)
        self.assertEqual(summary.key_lines, [])
        self.assertEqual(summary.suspected_tests, [])
