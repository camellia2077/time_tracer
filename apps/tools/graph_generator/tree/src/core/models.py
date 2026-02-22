from dataclasses import dataclass, field
from typing import List, Optional

@dataclass
class ProjectRecord:
    id: int
    parent_id: Optional[int]
    name: str

@dataclass
class TreeNode:
    id: int
    name: str
    duration_seconds: int = 0
    children: List["TreeNode"] = field(default_factory=list)
