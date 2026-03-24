from __future__ import annotations

from dataclasses import dataclass, field


@dataclass(slots=True)
class AutoFixAction:
    action_id: str
    kind: str
    file_path: str
    line: int
    col: int
    check: str
    status: str = "pending"
    reason: str = ""
    old_name: str = ""
    new_name: str = ""
    replacement: str = ""
    diff: str = ""
    edit_count: int = 0
    changed_files: list[str] = field(default_factory=list)


@dataclass(slots=True)
class TaskAutoFixResult:
    app_name: str
    task_id: str
    batch_id: str
    task_log: str
    source_file: str
    mode: str
    workspace: str
    source_scope: str | None
    applied: int = 0
    previewed: int = 0
    skipped: int = 0
    failed: int = 0
    action_count: int = 0
    actions: list[AutoFixAction] = field(default_factory=list)
    json_path: str = ""
    markdown_path: str = ""

    def exit_code(self, strict: bool = False) -> int:
        if strict and self.failed > 0:
            return 1
        if self.applied > 0 or self.previewed > 0:
            return 0
        if self.failed > 0:
            return 1
        return 1
