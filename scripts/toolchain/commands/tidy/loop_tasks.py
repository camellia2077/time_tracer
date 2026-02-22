import re
from pathlib import Path

from ...core.context import Context
from ...services import log_parser

TASK_ID_PATTERN = re.compile(r"task_(\d+)\.log$")


def task_sort_key(path: Path) -> tuple[int, str]:
    match = TASK_ID_PATTERN.match(path.name)
    if not match:
        return 10**9, path.name
    return int(match.group(1)), path.name


def next_task_path(tasks_dir: Path) -> Path | None:
    task_files = list(tasks_dir.rglob("task_*.log"))
    if not task_files:
        return None
    task_files.sort(key=task_sort_key)
    return task_files[0]


def task_id(path: Path) -> str:
    match = TASK_ID_PATTERN.match(path.name)
    if not match:
        return path.stem
    return match.group(1).zfill(3)


def classify_task(ctx: Context, task_path: Path) -> str:
    content = task_path.read_text(encoding="utf-8", errors="replace")
    diagnostics = log_parser.extract_diagnostics(content.splitlines())
    if not diagnostics:
        return "empty"

    checks = {
        diag.get("check", "").strip() for diag in diagnostics if diag.get("check", "").strip()
    }
    if not checks:
        return "manual"

    rename_check = ctx.config.rename.check_name
    if checks == {rename_check}:
        return "rename_only"
    return "manual"
