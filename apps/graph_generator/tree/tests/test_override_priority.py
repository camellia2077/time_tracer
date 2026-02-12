import sys
from pathlib import Path
import tempfile
import unittest


TEST_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = TEST_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))
if str(TEST_ROOT) not in sys.path:
    sys.path.insert(0, str(TEST_ROOT))

from core.config import TreeConfig
from run_tree import _build_parser, _resolve_runtime_overrides


class OverridePriorityTests(unittest.TestCase):
    def test_env_then_cli_override_priority(self) -> None:
        toml_text = """
[paths]
database = "config.sqlite3"
output_directory = "config_output"

[settings]
max_depth = 1
layout = "horizontal"
output_format = "png"
"""
        with tempfile.TemporaryDirectory() as temp_dir:
            config_path = Path(temp_dir) / "tree_config.toml"
            config_path.write_text(toml_text, encoding="utf-8")

            config = TreeConfig(str(config_path))
            config.apply_environment(
                {
                    "TREE_DB_PATH": "env.sqlite3",
                    "TREE_OUTPUT_DIR": "env_output",
                    "TREE_MAX_DEPTH": "2",
                    "TREE_LAYOUT": "vertical",
                }
            )
            config.apply_overrides(
                paths={"database": "cli.sqlite3"},
                settings={"max_depth": 5},
            )

            self.assertEqual(config.paths.database, "cli.sqlite3")
            self.assertEqual(config.paths.output_directory, "env_output")
            self.assertEqual(config.settings.max_depth, 5)
            self.assertEqual(config.settings.layout, "vertical")

    def test_cli_parser_respects_config_when_flags_omitted(self) -> None:
        parser = _build_parser()
        args = parser.parse_args(["tree"])

        root_path, max_depth, path_overrides, setting_overrides = _resolve_runtime_overrides(args)

        self.assertIsNone(root_path)
        self.assertIsNone(max_depth)
        self.assertEqual(path_overrides, {})
        self.assertEqual(setting_overrides, {})

    def test_cli_parser_collects_explicit_flags(self) -> None:
        parser = _build_parser()
        args = parser.parse_args(
            [
                "tree",
                "root_a",
                "--database",
                "cli.sqlite3",
                "--max-depth",
                "3",
                "--output-formats",
                "svg,png",
                "--output-json",
            ]
        )

        root_path, max_depth, path_overrides, setting_overrides = _resolve_runtime_overrides(args)

        self.assertEqual(root_path, "root_a")
        self.assertEqual(max_depth, 3)
        self.assertEqual(path_overrides["database"], "cli.sqlite3")
        self.assertEqual(setting_overrides["output_formats"], ["svg", "png"])
        self.assertTrue(setting_overrides["output_json"])


if __name__ == "__main__":
    unittest.main()
