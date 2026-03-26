from __future__ import annotations

import re
from pathlib import Path

from ..shared import tidy as tidy_shared
from .task_model import (
    TaskRecord,
    legacy_task_record_from_text,
    task_id_from_artifact_name,
    task_record_from_dict,
    toon_task_record_from_text,
)

TASK_FILE_PATTERN = re.compile(r"^task_(\d+)\.(?:json|log|toon)$")

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


def list_task_paths(tasks_dir: Path, batch_id: str | None = None) -> list[Path]:
    task_paths = _list_task_record_paths(tasks_dir, batch_id=batch_id)
    task_paths.sort(key=task_sort_key)
    return task_paths


def next_task_path(tasks_dir: Path, batch_id: str | None = None) -> Path | None:
    task_paths = list_task_paths(tasks_dir, batch_id=batch_id)
    if not task_paths:
        return None
    return task_paths[0]


def resolve_task_log_path(
    tasks_dir: Path,
    *,
    task_log_path: str | None = None,
    batch_id: str | None = None,
    task_id: str | None = None,
) -> Path:
    explicit_task_path = (task_log_path or "").strip()
    normalized_task_id = normalize_task_id(task_id)
    normalized_batch = tidy_shared.normalize_batch_name(batch_id, allow_none=True)

    if explicit_task_path:
        resolved = Path(explicit_task_path).expanduser()
        if not resolved.is_absolute():
            resolved = resolved.resolve()
        resolved = _canonical_task_path(resolved)
        if not resolved.exists():
            raise FileNotFoundError(f"task artifact not found: {resolved}")
        return resolved

    if normalized_task_id and normalized_batch:
        resolved = _resolve_task_path_in_batch(tasks_dir, normalized_batch, normalized_task_id)
        if resolved is None:
            raise FileNotFoundError(f"task record not found: {tasks_dir / normalized_batch / f'task_{normalized_task_id}.json'}")
        return resolved

    if normalized_task_id:
        matches = _find_task_paths_by_id(tasks_dir, normalized_task_id)
        matches.sort(key=task_sort_key)
        if not matches:
            raise FileNotFoundError(f"task_{normalized_task_id}.json not found under {tasks_dir}")
        if len(matches) > 1:
            joined = ", ".join(str(path.parent.name) for path in matches[:5])
            raise ValueError(
                f"task_{normalized_task_id}.json is ambiguous across batches: {joined}"
            )
        return matches[0]

    next_path = next_task_path(tasks_dir, batch_id=normalized_batch)
    if next_path is None:
        if normalized_batch:
            raise FileNotFoundError(f"no task records found under {tasks_dir / normalized_batch}")
        raise FileNotFoundError(f"no task records found under {tasks_dir}")
    return next_path


def resolve_task_json_path(
    tasks_dir: Path,
    *,
    task_log_path: str | None = None,
    batch_id: str | None = None,
    task_id: str | None = None,
) -> Path:
    resolved = resolve_task_log_path(
        tasks_dir,
        task_log_path=task_log_path,
        batch_id=batch_id,
        task_id=task_id,
    )
    if resolved.suffix.lower() == ".json":
        return resolved

    json_path = resolved.with_suffix(".json")
    if json_path.exists():
        return json_path

    # Auto-fix needs a stable machine-readable contract. `.log` and `.toon`
    # are human-facing views whose text layout can evolve, so we require the
    # canonical JSON artifact instead of reparsing presentation formats.
    raise FileNotFoundError(f"canonical task json not found: {json_path}")


def task_id(task_path: Path) -> str:
    resolved_id = task_id_from_artifact_name(task_path.name)
    if resolved_id is None:
        return task_path.stem
    return resolved_id


def load_task_record(task_path: Path) -> TaskRecord:
    resolved_path = _canonical_task_path(task_path)
    if resolved_path.suffix.lower() == ".json":
        payload = tidy_shared.read_json_dict(resolved_path)
        if payload is None:
            raise ValueError(f"invalid task record json: {resolved_path}")
        return task_record_from_dict(payload, fallback_path=resolved_path)

    content = resolved_path.read_text(encoding="utf-8", errors="replace")
    if resolved_path.suffix.lower() == ".toon":
        return toon_task_record_from_text(content, task_path=resolved_path)
    return legacy_task_record_from_text(content, task_path=resolved_path)


def parse_task_log(task_path: Path) -> ParsedTaskLog:
    return load_task_record(task_path)


def task_artifact_paths(task_path: Path) -> list[Path]:
    canonical = _canonical_task_path(task_path)
    base_path = canonical.with_suffix("")
    artifacts: list[Path] = []
    for suffix in (".json", ".log", ".toon"):
        candidate = base_path.with_suffix(suffix)
        if candidate.exists():
            artifacts.append(candidate)
    return artifacts


def _canonical_task_path(task_path: Path) -> Path:
    suffix = task_path.suffix.lower()
    if suffix == ".json" and task_path.exists():
        return task_path
    if suffix in {".log", ".toon"}:
        json_path = task_path.with_suffix(".json")
        if json_path.exists():
            return json_path
    if suffix == ".json":
        return task_path
    return task_path


def _list_task_record_paths(tasks_dir: Path, batch_id: str | None = None) -> list[Path]:
    search_root = tasks_dir / batch_id if batch_id else tasks_dir
    if not search_root.exists():
        return []
    matches: dict[str, Path] = {}
    for suffix in (".json", ".log", ".toon"):
        for task_path in search_root.rglob(f"task_*{suffix}"):
            canonical = _canonical_task_path(task_path)
            matches[str(canonical)] = canonical
    return list(matches.values())


def _resolve_task_path_in_batch(tasks_dir: Path, batch_id: str, task_id_value: str) -> Path | None:
    batch_dir = tasks_dir / batch_id
    if not batch_dir.exists() or not batch_dir.is_dir():
        return None
    for suffix in (".json", ".log", ".toon"):
        candidate = batch_dir / f"task_{task_id_value}{suffix}"
        if candidate.exists():
            return _canonical_task_path(candidate)
    return None


def _find_task_paths_by_id(tasks_dir: Path, task_id_value: str) -> list[Path]:
    matches: list[Path] = []
    for suffix in (".json", ".log", ".toon"):
        matches.extend(tasks_dir.rglob(f"task_{task_id_value}{suffix}"))
    normalized: dict[str, Path] = {}
    for match in matches:
        canonical = _canonical_task_path(match)
        normalized[str(canonical)] = canonical
    return list(normalized.values())
