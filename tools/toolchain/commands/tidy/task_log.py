from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

from ...services import log_parser
from ..shared import tidy as tidy_shared

TASK_FILE_PATTERN = re.compile(r"^task_(\d+)\.log$")


@dataclass(frozen=True, slots=True)
class ParsedTaskLog:
    task_path: Path
    task_id: str
    batch_id: str
    source_file: Path | None
    content: str
    diagnostics: list[dict]
    checks: list[str]


def normalize_task_id(task_id: str | None) -> str | None:
    raw = (task_id or "").strip()
    if not raw:
        return None
    if not raw.isdigit():
        raise ValueError("invalid --task-id. Use 11/011 style identifiers.")
    return str(int(raw)).zfill(3)


def task_sort_key(task_path: Path) -> tuple[int, str]:
    match = TASK_FILE_PATTERN.match(task_path.name)
    if not match:
        return 10**9, task_path.name
    return int(match.group(1)), task_path.name


def list_task_paths(tasks_dir: Path, batch_id: str | None = None) -> list[Path]:
    if batch_id:
        batch_dir = tasks_dir / tidy_shared.normalize_required_batch_name(batch_id)
        if not batch_dir.exists():
            return []
        task_paths = list(batch_dir.glob("task_*.log"))
    else:
        task_paths = list(tasks_dir.rglob("task_*.log"))
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
        if not resolved.exists():
            raise FileNotFoundError(f"task log not found: {resolved}")
        return resolved

    if normalized_task_id and normalized_batch:
        resolved = tasks_dir / normalized_batch / f"task_{normalized_task_id}.log"
        if not resolved.exists():
            raise FileNotFoundError(f"task log not found: {resolved}")
        return resolved

    if normalized_task_id:
        matches = list(tasks_dir.rglob(f"task_{normalized_task_id}.log"))
        matches.sort(key=task_sort_key)
        if not matches:
            raise FileNotFoundError(
                f"task_{normalized_task_id}.log not found under {tasks_dir}"
            )
        if len(matches) > 1:
            joined = ", ".join(str(path.parent.name) for path in matches[:5])
            raise ValueError(
                f"task_{normalized_task_id}.log is ambiguous across batches: {joined}"
            )
        return matches[0]

    next_path = next_task_path(tasks_dir, batch_id=normalized_batch)
    if next_path is None:
        if normalized_batch:
            raise FileNotFoundError(f"no task logs found under {tasks_dir / normalized_batch}")
        raise FileNotFoundError(f"no task logs found under {tasks_dir}")
    return next_path


def task_id(task_path: Path) -> str:
    match = TASK_FILE_PATTERN.match(task_path.name)
    if not match:
        return task_path.stem
    return match.group(1).zfill(3)


def parse_task_log(task_path: Path) -> ParsedTaskLog:
    content = task_path.read_text(encoding="utf-8", errors="replace")
    diagnostics = log_parser.extract_diagnostics(content.splitlines())
    source_file: Path | None = None
    for line in content.splitlines():
        if line.startswith("File: "):
            raw_source = line[len("File: ") :].strip()
            if raw_source:
                source_file = Path(raw_source)
            break
    checks: list[str] = []
    for diagnostic in diagnostics:
        check_name = str(diagnostic.get("check", "")).strip()
        if check_name and check_name not in checks:
            checks.append(check_name)
    return ParsedTaskLog(
        task_path=task_path,
        task_id=task_id(task_path),
        batch_id=task_path.parent.name if task_path.parent else "",
        source_file=source_file,
        content=content,
        diagnostics=diagnostics,
        checks=checks,
    )
