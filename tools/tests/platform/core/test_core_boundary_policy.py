from pathlib import Path
from unittest import TestCase


class TestCoreBoundaryPolicy(TestCase):
    def test_core_targets_declares_compat_and_aggregate_guards(self):
        repo_root = Path(__file__).resolve().parents[4]
        core_targets = (
            repo_root / "apps" / "tracer_core_shell" / "cmake" / "CoreTargets.cmake"
        ).read_text(encoding="utf-8")

        self.assertIn("application/dto/compat/", core_targets)
        self.assertIn("application/aggregate_runtime/", core_targets)
        self.assertIn("application/dto/core_requests\\\\.hpp", core_targets)
        self.assertIn("application/dto/core_responses\\\\.hpp", core_targets)
        self.assertIn("application/interfaces/i_report_", core_targets)
        self.assertIn("application/use_cases/tracer_core_runtime\\\\.hpp", core_targets)
        self.assertIn('"api/c_api/tracer_core_c_api.cpp"', core_targets)
        self.assertNotIn('"api/c_api/tracer_core_c_api_crypto.cpp"', core_targets)
