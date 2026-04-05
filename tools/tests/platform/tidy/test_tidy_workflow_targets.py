from pathlib import Path
from unittest import TestCase


class TestTidyWorkflowTargets(TestCase):
    def test_core_family_prebuild_targets_use_active_reporting_target(self):
        repo_root = Path(__file__).resolve().parents[4]
        workflow_toml = (
            repo_root / "tools" / "toolchain" / "config" / "workflow.toml"
        ).read_text(encoding="utf-8")

        self.assertIn('"tc_infra_reporting_lib"', workflow_toml)
        self.assertNotIn('"tc_infra_reports_lib"', workflow_toml)
        self.assertNotIn('"tc_infra_full_lib"', workflow_toml)
