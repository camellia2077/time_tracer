import sys
import tempfile
import unittest
from pathlib import Path


HEATMAP_ROOT = Path(__file__).resolve().parents[1]
if str(HEATMAP_ROOT) not in sys.path:
    sys.path.insert(0, str(HEATMAP_ROOT))

from heatmap_app.core.config import AppConfig  # noqa: E402


_COLORS_TOML = """
[COLOR_PALETTES]
GITHUB_GREEN_LIGHT = ["#ebedf0", "#9be9a8", "#40c463"]

[SINGLE_COLORS]
ORANGE = "#f97316"
"""


class AppConfigSourceModeTests(unittest.TestCase):
    def _build_config(self, temp_dir: Path, source_block: str) -> Path:
        db_path = temp_dir / "time_data.sqlite3"
        db_path.touch()

        config_text = f"""
[database]
path = "{db_path.as_posix()}"

{source_block}

[heatmap]
year = 2026
project_names = ["study"]
color_palette = "GITHUB_GREEN_LIGHT"
over_12_hours_color = "ORANGE"

[boolean_heatmaps]
enabled_reports = ["sleep"]

[boolean_heatmaps.outputs]
sleep = "sleep_heatmap"
"""
        config_dir = temp_dir / "configs"
        config_dir.mkdir(parents=True, exist_ok=True)
        (config_dir / "heatmap_config.toml").write_text(
            config_text, encoding="utf-8"
        )
        (config_dir / "heatmap_colors.toml").write_text(
            _COLORS_TOML, encoding="utf-8"
        )
        return config_dir

    def test_core_contract_mode_parses(self):
        with tempfile.TemporaryDirectory() as temp:
            temp_dir = Path(temp)
            config_dir = self._build_config(
                temp_dir,
                """
[source]
mode = "core_contract"
core_cli_path = "C:/tools/time_tracer_cli.exe"
core_timeout_seconds = 35
allow_sql_fallback = false
""",
            )

            config = AppConfig(config_dir)
            self.assertEqual(config.source_mode, "core_contract")
            self.assertEqual(config.core_cli_path, "C:/tools/time_tracer_cli.exe")
            self.assertEqual(config.core_timeout_seconds, 35)
            self.assertFalse(config.allow_sql_fallback)

    def test_unknown_mode_falls_back_to_sqlite(self):
        with tempfile.TemporaryDirectory() as temp:
            temp_dir = Path(temp)
            config_dir = self._build_config(
                temp_dir,
                """
[source]
mode = "unknown_mode"
""",
            )

            config = AppConfig(config_dir)
            self.assertEqual(config.source_mode, "sqlite")


if __name__ == "__main__":
    unittest.main()

