from typing import Dict, List

from core.models import ProjectRecord, TreeNode


class TreeBuilder:
    """Builds a tree structure from project records and duration map."""

    def build(self, projects: List[ProjectRecord], durations: Dict[int, int]) -> List[TreeNode]:
        nodes = {
            record.id: TreeNode(record.id, record.name, durations.get(record.id, 0))
            for record in projects
        }
        roots: List[TreeNode] = []

        for record in projects:
            node = nodes[record.id]
            if record.parent_id is None or record.parent_id not in nodes:
                roots.append(node)
            else:
                nodes[record.parent_id].children.append(node)

        self._sort_tree(roots)
        return roots

    def _sort_tree(self, nodes: List[TreeNode]) -> None:
        nodes.sort(key=lambda item: item.name)
        for node in nodes:
            if node.children:
                self._sort_tree(node.children)
