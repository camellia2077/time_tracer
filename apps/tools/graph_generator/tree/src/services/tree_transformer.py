from collections import deque
from typing import List

from core.models import TreeNode


class TreeTransformer:
    """Applies filtering, pruning, and aggregation over tree nodes."""

    def filter_roots_by_path(self, roots: List[TreeNode], root_path: str) -> List[TreeNode]:
        targets: List[TreeNode] = []
        queue = deque((root, root.name) for root in roots)

        while queue:
            node, path = queue.popleft()
            if path == root_path:
                targets.append(node)
            for child in node.children:
                queue.append((child, f"{path}_{child.name}"))

        return targets

    def prune_to_depth(self, roots: List[TreeNode], max_depth: int) -> List[TreeNode]:
        if max_depth < 0:
            return roots
        return [self._prune(root, 0, max_depth) for root in roots]

    def accumulate_durations(self, roots: List[TreeNode]) -> None:
        def post_order(node: TreeNode) -> int:
            total = node.duration_seconds
            for child in node.children:
                total += post_order(child)
            node.duration_seconds = total
            return total

        for root in roots:
            post_order(root)

    def _prune(self, node: TreeNode, depth: int, max_depth: int) -> TreeNode:
        pruned = TreeNode(node.id, node.name, node.duration_seconds)
        if depth < max_depth:
            pruned.children = [self._prune(child, depth + 1, max_depth) for child in node.children]
        return pruned
