from __future__ import annotations

import re
from pathlib import Path

from ...shared import tidy as tidy_shared
from .task_model import TaskRecord, task_id_from_artifact_name, task_record_from_dict

TASK_FILE_PATTERN = re.compile(r"^task_(\d+)\.json$")
ParsedTaskLog = TaskRecord


def normalize_task_id(task_id: str | None) -> str | None:
    raw = (task_id or "").strip()
    if not raw:
        return None
    if not raw.isdigit():
        raise ValueError("invalid --task-id. Use 11/011 style identifiers.")
    return str(int(raw)).zfill(3)


def task_sort_key(task_path: Path) -> tuple[int, str]:
    resolved_id = task_id(task_path)
    if not resolved_id:
        return 10**9, task_path.name
    return int(resolved_id), task_path.name


def list_task_paths(tasks_dir: Path, cluster_id: str | None = None) -> list[Path]:
    search_root = tasks_dir / "clusters"
    if cluster_id:
        search_root = search_root / cluster_id
    if not search_root.exists():
        return []
    task_paths = list(search_root.rglob("task_*.json"))
    task_paths = [path for path in task_paths if TASK_FILE_PATTERN.match(path.name)]
    task_paths.sort(key=task_sort_key)
    return task_paths


def next_task_path(tasks_dir: Path, cluster_id: str | None = None) -> Path | None:
    paths = list_task_paths(tasks_dir, cluster_id=cluster_id)
    return paths[0] if paths else None


def resolve_task_log_path(
    tasks_dir: Path,
    *,
    task_log_path: str | None = None,
    cluster_id: str | None = None,
    task_id: str | None = None,
) -> Path:
    explicit_task_path = (task_log_path or "").strip()
    normalized_task_id = normalize_task_id(task_id)

    if explicit_task_path:
        resolved = Path(explicit_task_path).expanduser()
        if not resolved.is_absolute():
            resolved = resolved.resolve()
        if resolved.suffix.lower() != ".json":
            raise ValueError(f"task input must be a canonical .json record: {resolved}")
        if not resolved.exists():
            raise FileNotFoundError(f"task record not found: {resolved}")
        return resolved

    if normalized_task_id and cluster_id:
        resolved = tasks_dir / "clusters" / cluster_id / f"task_{normalized_task_id}.json"
        if not resolved.exists():
            raise FileNotFoundError(f"task record not found: {resolved}")
        return resolved

    if normalized_task_id:
        matches = [
            path for path in list_task_paths(tasks_dir)
            if task_id(path) == normalized_task_id
        ]
        if not matches:
            raise FileNotFoundError(f"task_{normalized_task_id}.json not found under {tasks_dir}")
        if len(matches) > 1:
            joined = ", ".join(path.parent.name for path in matches[:5])
            raise ValueError(
                f"task_{normalized_task_id}.json is ambiguous across clusters: {joined}"
            )
        return matches[0]

    next_path = next_task_path(tasks_dir, cluster_id=cluster_id)
    if next_path is None:
        if cluster_id:
            raise FileNotFoundError(
                f"no task records found under {tasks_dir / 'clusters' / cluster_id}"
            )
        raise FileNotFoundError(f"no task records found under {tasks_dir}")
    return next_path


def resolve_task_json_path(
    tasks_dir: Path,
    *,
    task_log_path: str | None = None,
    cluster_id: str | None = None,
    task_id: str | None = None,
) -> Path:
    return resolve_task_log_path(
        tasks_dir,
        task_log_path=task_log_path,
        cluster_id=cluster_id,
        task_id=task_id,
    )


def task_id(task_path: Path) -> str:
    return task_id_from_artifact_name(task_path.name) or task_path.stem


def load_task_record(task_path: Path) -> TaskRecord:
    if task_path.suffix.lower() != ".json":
        raise ValueError(f"task input must be a canonical .json record: {task_path}")
    payload = tidy_shared.read_json_dict(task_path)
    if payload is None:
        raise ValueError(f"invalid task record json: {task_path}")
    return task_record_from_dict(payload, fallback_path=task_path)


def parse_task_log(task_path: Path) -> ParsedTaskLog:
    return load_task_record(task_path)


def task_view_paths(task_path: Path) -> list[Path]:
    """Return the canonical task record and its currently rendered views."""
    canonical = task_path.with_suffix(".json")
    return [
        candidate
        for candidate in (
            canonical,
            canonical.with_suffix(".log"),
            canonical.with_suffix(".toon"),
        )
        if candidate.exists()
    ]
