from pathlib import Path
from typing import List


def cleanup_task_logs(tasks_dir: Path, task_ids: List[str]) -> int:
    """
    Delete task logs matching the provided IDs.
    Returns the count of deleted files.
    """
    if not tasks_dir.exists():
        print(f"Error: Tasks directory not found at {tasks_dir}")
        return 0

    print(f"Cleaning logs for IDs: {', '.join(task_ids)}")
    deleted_count = 0
    for task_id in task_ids:
        pattern = f"task_{task_id}.log"
        files = list(tasks_dir.glob(pattern))
        if not files:
            files = list(tasks_dir.glob(f"task_{task_id}_*.log"))

        if not files:
            print(f"  [Warn] No logs found for ID {task_id}")
            continue

        for log_file in files:
            try:
                log_file.unlink()
                print(f"  [Deleted] {log_file.name}")
                deleted_count += 1
            except Exception as e:
                print(f"  [Error] Failed to delete {log_file.name}: {e}")

    return deleted_count


def list_tasks(tasks_dir: Path):
    """
    List all available task logs.
    """
    if not tasks_dir.exists():
        return

    logs = sorted(tasks_dir.glob("task_*.log"))
    if not logs:
        print("No task logs found.")
        return

    print(f"Available tasks in {tasks_dir}:")
    for log in logs:
        print(f"  - {log.name}")
