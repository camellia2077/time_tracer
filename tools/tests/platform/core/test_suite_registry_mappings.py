import sys
from pathlib import Path
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[4]
TOOLS_DIR = REPO_ROOT / "tools"

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.services.suite_registry import (  # noqa: E402
    needs_suite_build,
    resolve_result_output_name,
    resolve_suite_build_app,
    resolve_suite_name,
    resolve_suite_runner_name,
)


class TestSuiteRegistryMappings(TestCase):
    def test_rust_cli_maps_to_windows_rust_cli_suite(self):
        self.assertEqual(resolve_suite_name("tracer_windows_rust_cli"), "tracer_windows_rust_cli")
        self.assertEqual(
            resolve_suite_runner_name("tracer_windows_rust_cli"),
            "artifact_windows_cli",
        )
        self.assertEqual(
            resolve_suite_build_app("tracer_windows_rust_cli"),
            "tracer_windows_rust_cli",
        )
        self.assertFalse(needs_suite_build("tracer_windows_rust_cli"))

    def test_result_output_name_uses_canonical_artifact_targets(self):
        self.assertEqual(resolve_result_output_name("tracer_core"), "artifact_windows_cli")
        self.assertEqual(
            resolve_result_output_name("tracer_windows_rust_cli"),
            "artifact_windows_cli",
        )
        self.assertEqual(resolve_result_output_name("tracer_android"), "artifact_android")
        self.assertEqual(resolve_result_output_name("log_generator"), "artifact_log_generator")
        self.assertEqual(resolve_result_output_name("unknown_app"), "unknown_app")
