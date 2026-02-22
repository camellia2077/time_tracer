import sys
from pathlib import Path
import unittest


TEST_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = TEST_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

from core.models import TreeNode
from services.tree_transformer import TreeTransformer


class TreeTransformerTests(unittest.TestCase):
    def test_filter_roots_by_path_finds_nested_target(self) -> None:
        target = TreeNode(id=3, name="target", duration_seconds=7)
        branch = TreeNode(id=2, name="branch", duration_seconds=5, children=[target])
        root = TreeNode(id=1, name="root", duration_seconds=3, children=[branch])
        other = TreeNode(id=10, name="other", duration_seconds=1)

        result = TreeTransformer().filter_roots_by_path([root, other], "root_branch_target")

        self.assertEqual(len(result), 1)
        self.assertIs(result[0], target)

    def test_prune_to_depth_limits_children(self) -> None:
        leaf = TreeNode(id=3, name="leaf", duration_seconds=7)
        child = TreeNode(id=2, name="child", duration_seconds=5, children=[leaf])
        root = TreeNode(id=1, name="root", duration_seconds=3, children=[child])

        pruned_roots = TreeTransformer().prune_to_depth([root], max_depth=1)

        self.assertEqual(len(pruned_roots), 1)
        self.assertEqual(pruned_roots[0].name, "root")
        self.assertEqual(len(pruned_roots[0].children), 1)
        self.assertEqual(pruned_roots[0].children[0].name, "child")
        self.assertEqual(pruned_roots[0].children[0].children, [])

    def test_accumulate_durations_sums_descendants(self) -> None:
        leaf1 = TreeNode(id=3, name="leaf1", duration_seconds=7)
        leaf2 = TreeNode(id=4, name="leaf2", duration_seconds=11)
        child = TreeNode(id=2, name="child", duration_seconds=5, children=[leaf1, leaf2])
        root = TreeNode(id=1, name="root", duration_seconds=3, children=[child])

        TreeTransformer().accumulate_durations([root])

        self.assertEqual(leaf1.duration_seconds, 7)
        self.assertEqual(leaf2.duration_seconds, 11)
        self.assertEqual(child.duration_seconds, 23)
        self.assertEqual(root.duration_seconds, 26)


if __name__ == "__main__":
    unittest.main()
