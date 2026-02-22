import json
import sys
from pathlib import Path
import tempfile
import unittest


TEST_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = TEST_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

from core.models import TreeNode
from services.tree_exporters import ExportRequest, HtmlTreeExporter, JsonTreeExporter


class TreeExportersDurationTests(unittest.TestCase):
    def test_json_exporter_includes_days_and_hm_text(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            exporter = JsonTreeExporter(temp_dir)
            node = TreeNode(id=1, name="root", duration_seconds=90061)
            request = ExportRequest(roots=[node], title="demo", output_name="tree")

            json_text = exporter._build_json_text(request)
            payload = json.loads(json_text)
            root = payload["roots"][0]

            self.assertEqual(root["duration_text"], "25h 1m")
            self.assertEqual(root["duration_days_text"], "1.04d")

    def test_html_exporter_embedded_json_includes_days_text(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            exporter = HtmlTreeExporter(temp_dir)
            node = TreeNode(id=1, name="root", duration_seconds=3600)
            request = ExportRequest(roots=[node], title="demo", output_name="tree")

            json_text = exporter._build_json_text(request)
            payload = json.loads(json_text)
            root = payload["roots"][0]

            self.assertEqual(root["duration_text"], "1h 0m")
            self.assertEqual(root["duration_days_text"], "0.04d")


if __name__ == "__main__":
    unittest.main()
