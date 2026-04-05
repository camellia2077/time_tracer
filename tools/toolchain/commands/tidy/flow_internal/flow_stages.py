import json
import re
from pathlib import Path

from ....core.context import Context
from ...cmd_quality.verify import VerifyCommand
from ..clean import CleanCommand
from ..tasking.task_log import list_task_paths as list_task_record_paths, load_task_record

TASK_ID_PATTERN = re.compile(r"task_(\d+)\.(?:json|log|toon)$")


def count_rename_candidates(build_tidy_dir: Path) -> int:
    candidates_path = build_tidy_dir / "rename" / "rename_candidates.json"
    if not candidates_path.exists():
        return 0
    try:
        payload = json.loads(candidates_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return 0
    candidates = payload.get("candidates", [])
    if isinstance(candidates, list):
        return len(candidates)
    return 0


def run_suite_verify(
    ctx: Context,
    app_name: str,
    build_dir_name: str,
    concise: bool,
) -> int:
    return VerifyCommand(ctx).run_tests(
        app_name=app_name,
        build_dir_name=build_dir_name,
        concise=concise,
    )


def has_task_logs(tasks_dir: Path) -> bool:
    return bool(list_task_record_paths(tasks_dir))


def list_task_paths(tasks_dir: Path) -> list[Path]:
    return list_task_record_paths(tasks_dir)


def list_task_ids(tasks_dir: Path) -> list[str]:
    task_ids: list[str] = []
    for task_path in list_task_paths(tasks_dir):
        current_task_id = task_id(task_path)
        if current_task_id:
            task_ids.append(current_task_id)
    return task_ids


def clean_empty_tasks(
    ctx: Context,
    app_name: str,
    tasks_dir: Path,
    tidy_build_dir_name: str | None = None,
) -> list[str]:
    if not tasks_dir.exists():
        return []

    empty_task_ids: list[str] = []
    for task_path in list_task_paths(tasks_dir):
        try:
            record = load_task_record(task_path)
        except (OSError, ValueError):
            continue
        if record.diagnostics:
            continue
        current_task_id = task_id(task_path)
        if current_task_id:
            empty_task_ids.append(current_task_id)

    if not empty_task_ids:
        return []

    clean_ret = CleanCommand(ctx).execute(
        app_name,
        empty_task_ids,
        tidy_build_dir_name=tidy_build_dir_name,
    )
    if clean_ret != 0:
        return []
    return empty_task_ids


def task_sort_key(task_path: Path) -> tuple[int, str]:
    match = TASK_ID_PATTERN.match(task_path.name)
    if not match:
        return 10**9, task_path.name
    return int(match.group(1)), task_path.name


def task_id(task_path: Path) -> str | None:
    match = TASK_ID_PATTERN.match(task_path.name)
    if not match:
        return None
    return match.group(1).zfill(3)
