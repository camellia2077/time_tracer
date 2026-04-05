import re
from pathlib import Path

from ....core.context import Context
from ..tasking.task_log import list_task_paths, load_task_record

TASK_ID_PATTERN = re.compile(r"task_(\d+)\.(?:json|log|toon)$")


def task_sort_key(path: Path) -> tuple[int, str]:
    match = TASK_ID_PATTERN.match(path.name)
    if not match:
        return 10**9, path.name
    return int(match.group(1)), path.name


def next_task_path(tasks_dir: Path) -> Path | None:
    task_files = list_task_paths(tasks_dir)
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
    try:
        record = load_task_record(task_path)
    except (OSError, ValueError):
        return "empty"
    if not record.diagnostics:
        return "empty"

    checks = {
        diagnostic.check.strip()
        for diagnostic in record.diagnostics
        if diagnostic.check.strip()
    }
    if not checks:
        return "manual"

    rename_check = ctx.config.rename.check_name
    if checks == {rename_check}:
        return "rename_only"
    return "manual"
