import sys
from pathlib import Path
import unittest


TEST_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = TEST_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

from core.models import ProjectRecord
from services.tree_builder import TreeBuilder


class TreeBuilderTests(unittest.TestCase):
    def test_build_creates_sorted_hierarchy(self) -> None:
        projects = [
            ProjectRecord(id=10, parent_id=None, name="beta"),
            ProjectRecord(id=20, parent_id=10, name="child_z"),
            ProjectRecord(id=30, parent_id=10, name="child_a"),
            ProjectRecord(id=40, parent_id=None, name="alpha"),
            ProjectRecord(id=50, parent_id=9999, name="orphan"),
        ]
        durations = {
            10: 120,
            20: 30,
            30: 10,
            40: 5,
            50: 15,
        }

        roots = TreeBuilder().build(projects, durations)

        self.assertEqual([node.name for node in roots], ["alpha", "beta", "orphan"])
        beta = roots[1]
        self.assertEqual([child.name for child in beta.children], ["child_a", "child_z"])
        self.assertEqual(beta.duration_seconds, 120)
        self.assertEqual(beta.children[0].duration_seconds, 10)
        self.assertEqual(beta.children[1].duration_seconds, 30)


if __name__ == "__main__":
    unittest.main()
