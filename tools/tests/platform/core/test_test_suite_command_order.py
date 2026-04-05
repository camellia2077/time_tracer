from __future__ import annotations

import sys
from pathlib import Path
from unittest import TestCase


REPO_ROOT = Path(__file__).resolve().parents[4]
TEST_ROOT = REPO_ROOT / "test"
if str(TEST_ROOT) not in sys.path:
    sys.path.insert(0, str(TEST_ROOT))

from framework.core.conf.loader import load_config


SUITE_CONFIG = REPO_ROOT / "test" / "suites" / "tracer_windows_rust_cli" / "config.toml"


def _first_stage_indexes(stage_names: list[str]) -> dict[str, int]:
    indexes: dict[str, int] = {}
    for idx, stage_name in enumerate(stage_names):
        indexes.setdefault(stage_name, idx)
    return indexes


class TestSuiteCommandOrder(TestCase):
    def test_reporting_stages_preserve_include_order_before_exchange(self):
        config = load_config(SUITE_CONFIG, build_dir_name="build_fast")
        stage_names = [command.stage for command in config.commands]
        stage_indexes = _first_stage_indexes(stage_names)

        self.assertIn("report", stage_indexes)
        self.assertIn("report-export", stage_indexes)
        self.assertIn("exchange", stage_indexes)
        self.assertLess(
            stage_indexes["report"],
            stage_indexes["exchange"],
            "report stage should follow tests.toml include order and run before exchange",
        )
        self.assertLess(
            stage_indexes["report-export"],
            stage_indexes["exchange"],
            "report-export stage should follow tests.toml include order and run before exchange",
        )
