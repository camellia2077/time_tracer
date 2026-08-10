import sys
from pathlib import Path
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[4]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.commands.cmd_workflow.config_migrate_planner import (  # noqa: E402
    extract_insights_paths,
)


class TestConfigMigratePlanner(TestCase):
    def test_rejects_removed_reports_table_as_unknown(self) -> None:
        with self.assertRaisesRegex(RuntimeError, r"Unknown config table .*'reports'"):
            extract_insights_paths(
                {
                    "reports": {
                        "markdown": {"root": "reports/markdown"},
                    }
                },
                Path("config.toml"),
            )

    def test_accepts_only_insights_format_tables(self) -> None:
        result = extract_insights_paths(
            {
                "insights": {
                    "markdown": {
                        "root": "insights/markdown",
                        "default_locale": "en",
                        "supported_locales": ["en", "zh", "ja"],
                    },
                    "latex": {"root": "insights/latex"},
                    "typst": {"root": "insights/typst"},
                }
            },
            Path("config.toml"),
        )

        self.assertEqual(result["markdown"]["root"], "insights/markdown")
        self.assertEqual(result["latex"]["root"], "insights/latex")
        self.assertEqual(result["typst"]["root"], "insights/typst")

