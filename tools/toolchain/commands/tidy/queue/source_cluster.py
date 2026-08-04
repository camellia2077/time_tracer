from __future__ import annotations

import os
from dataclasses import dataclass
import hashlib
import re
from pathlib import Path

from .task_log import list_task_paths, load_task_record
from .task_model import TaskRecord


@dataclass(frozen=True, slots=True)
class SourceTaskCluster:
    """All pending task artifacts whose primary source file is the same."""

    source_file: str
    source_key: str
    task_paths: tuple[Path, ...]
    task_records: tuple[TaskRecord, ...]

    @property
    def task_ids(self) -> tuple[str, ...]:
        return tuple(record.task_id for record in self.task_records)

    @property
    def cluster_id(self) -> str:
        return self.task_records[0].cluster_id if self.task_records else ""


def source_file_key(source_file: str | Path) -> str:
    """Return a stable comparison key for a source path on the host platform."""

    raw_path = Path(str(source_file).strip()).expanduser()
    try:
        normalized = raw_path.resolve(strict=False)
    except OSError:
        normalized = raw_path
    return os.path.normcase(os.path.normpath(str(normalized)))


def collect_source_clusters(tasks_dir: Path) -> tuple[SourceTaskCluster, ...]:
    """Collect pending task records into stable source-file clusters.

    Task artifacts remain the source of truth for queue membership. The
    returned order follows the queue's task ordering, with each source file
    represented once. Records inside a cluster retain that same stable order.
    """

    grouped: dict[str, tuple[str, list[Path], list[TaskRecord]]] = {}
    for task_path in list_task_paths(tasks_dir):
        record = load_task_record(task_path)
        source_file = str(record.source_file or "").strip()
        if not source_file:
            raise ValueError(f"task has no source_file: {task_path}")
        key = source_file_key(source_file)
        entry = grouped.setdefault(key, (source_file, [], []))
        entry[1].append(task_path)
        entry[2].append(record)

    clusters: list[SourceTaskCluster] = []
    for key, (source_file, task_paths, task_records) in grouped.items():
        clusters.append(
            SourceTaskCluster(
                source_file=source_file,
                source_key=key,
                task_paths=tuple(task_paths),
                task_records=tuple(task_records),
            )
        )
    return tuple(clusters)


def resolve_source_cluster(
    tasks_dir: Path,
    *,
    source_file: str | Path,
) -> SourceTaskCluster | None:
    """Resolve one source cluster from the current pending queue."""

    wanted_key = source_file_key(source_file)
    for cluster in collect_source_clusters(tasks_dir):
        if cluster.source_key == wanted_key:
            return cluster
    return None


def cluster_id_for_source(source_file: str | Path) -> str:
    """Return a stable, readable directory name for one source file."""

    raw = str(source_file).strip().replace("\\", "/")
    key = source_file_key(raw)
    digest = hashlib.sha1(key.encode("utf-8", errors="replace")).hexdigest()[:10]
    filename = raw.rsplit("/", 1)[-1]
    slug = re.sub(r"[^A-Za-z0-9._-]+", "_", filename).strip("._-")
    slug = slug[:80] if slug else "source"
    return f"{slug}_{digest}"


def resolve_task_source_cluster(
    tasks_dir: Path,
    task_path: Path,
) -> SourceTaskCluster:
    """Resolve the current cluster containing one task artifact."""

    record = load_task_record(task_path)
    cluster = resolve_source_cluster(tasks_dir, source_file=record.source_file)
    if cluster is None:
        raise FileNotFoundError(
            f"no pending source cluster for task source: {record.source_file}"
        )
    return cluster


__all__ = [
    "SourceTaskCluster",
    "collect_source_clusters",
    "resolve_source_cluster",
    "resolve_task_source_cluster",
    "source_file_key",
    "cluster_id_for_source",
]
