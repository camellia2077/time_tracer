from __future__ import annotations

from pathlib import Path

from ..shared import tidy as tidy_shared

QUEUE_STATE_FILE = "queue_state.json"


def queue_state_path(tasks_dir: Path) -> Path:
    return tasks_dir / QUEUE_STATE_FILE


def read_queue_state(tasks_dir: Path) -> dict | None:
    return tidy_shared.read_json_dict(queue_state_path(tasks_dir))


def read_queue_generation(tasks_dir: Path) -> int | None:
    payload = read_queue_state(tasks_dir)
    if not isinstance(payload, dict):
        return None
    raw_value = payload.get("queue_generation")
    if isinstance(raw_value, int):
        return raw_value
    if isinstance(raw_value, str):
        try:
            return int(raw_value)
        except ValueError:
            return None
    return None


def next_queue_generation(tasks_dir: Path) -> int:
    current_generation = read_queue_generation(tasks_dir)
    if current_generation is None or current_generation < 0:
        return 1
    return current_generation + 1


def write_queue_state(
    tasks_dir: Path,
    *,
    queue_generation: int,
    task_count: int,
    batch_count: int,
    task_view: str,
) -> Path:
    path = queue_state_path(tasks_dir)
    tidy_shared.write_json_dict(
        path,
        {
            "queue_generation": int(queue_generation),
            "task_count": int(task_count),
            "batch_count": int(batch_count),
            "task_view": str(task_view or "").strip(),
        },
    )
    return path
