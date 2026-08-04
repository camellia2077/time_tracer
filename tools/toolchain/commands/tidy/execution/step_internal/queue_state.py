from __future__ import annotations

from pathlib import Path

from ...queue.task_log import list_task_paths, load_task_record


def build_queue_head(tasks_dir: Path) -> dict | None:
    pending_logs = list_task_paths(tasks_dir)
    if not pending_logs:
        return None
    head_path = pending_logs[0]
    parsed = load_task_record(head_path)
    return {
        "task_id": parsed.task_id,
        "cluster_id": parsed.cluster_id,
        "source_file": parsed.source_file or str(head_path),
        "task_log": str(head_path),
        "checks": list(parsed.checks),
    }
