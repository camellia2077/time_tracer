import sys
from pathlib import Path
import tempfile
import unittest


TEST_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = TEST_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

from core.config import TreeConfig


class TreeConfigValidationTests(unittest.TestCase):
    def test_validation_normalizes_invalid_values(self) -> None:
        toml_text = """
[paths]
database = " data.sqlite3 "
output_directory = " "

[settings]
root_path = " "
max_depth = -5
layout = "diagonal"
split_roots = true
output_name = " "
title = " "
output_format = "bmp"
output_formats = ["png", "bad", "svg", "png"]
output_html = false
output_json = true
output_images = true

[style]
figure_width = 0
figure_height = -1
dpi = 0
node_size = -2
label_font_size = 0
horizontal_spacing = 0
vertical_spacing = -1
root_node_scale = 0
min_node_scale = 0
label_root_scale = 0
label_min_scale = 0
"""
        with tempfile.TemporaryDirectory() as temp_dir:
            config_path = Path(temp_dir) / "tree_config.toml"
            config_path.write_text(toml_text, encoding="utf-8")
            config = TreeConfig(str(config_path))

            self.assertEqual(config.paths.database, "data.sqlite3")
            self.assertEqual(config.paths.output_directory, "")
            self.assertEqual(config.settings.max_depth, -1)
            self.assertEqual(config.settings.layout, "horizontal")
            self.assertEqual(config.settings.output_name, "project_tree")
            self.assertEqual(config.settings.title, "Project Tree")
            self.assertEqual(config.settings.output_format, "png")
            self.assertEqual(config.settings.output_formats, ["png", "svg"])
            self.assertEqual(config.style.figure_width, 16.0)
            self.assertEqual(config.style.figure_height, 10.0)
            self.assertEqual(config.style.dpi, 300)
            self.assertEqual(config.style.node_size, 280.0)


if __name__ == "__main__":
    unittest.main()
