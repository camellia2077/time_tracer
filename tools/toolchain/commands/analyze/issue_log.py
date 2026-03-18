from __future__ import annotations

import re
from pathlib import Path

from ..shared import tidy as shared_json
from .issue_model import (
    AnalyzeIssue,
    issue_from_dict,
    issue_id_from_artifact_name,
    toon_issue_from_text,
)

ISSUE_FILE_PATTERN = re.compile(r"^issue_(\d+)\.(?:json|toon)$")


def normalize_issue_id(issue_id: str | None) -> str | None:
    raw = (issue_id or "").strip()
    if not raw:
        return None
    if not raw.isdigit():
        raise ValueError("invalid --issue-id. Use 11/011 style identifiers.")
    return str(int(raw)).zfill(3)


def issue_sort_key(issue_path: Path) -> tuple[int, str]:
    resolved_id = issue_id(issue_path)
    if not resolved_id:
        return 10**9, issue_path.name
    return int(resolved_id), issue_path.name


def list_issue_paths(issues_dir: Path, batch_id: str | None = None) -> list[Path]:
    issue_paths = _list_issue_record_paths(issues_dir, batch_id=batch_id)
    issue_paths.sort(key=issue_sort_key)
    return issue_paths


def next_issue_path(issues_dir: Path, batch_id: str | None = None) -> Path | None:
    issue_paths = list_issue_paths(issues_dir, batch_id=batch_id)
    if not issue_paths:
        return None
    return issue_paths[0]


def resolve_issue_path(
    issues_dir: Path,
    *,
    issue_path: str | None = None,
    batch_id: str | None = None,
    issue_id_value: str | None = None,
) -> Path:
    explicit_issue_path = (issue_path or "").strip()
    normalized_issue_id = normalize_issue_id(issue_id_value)
    normalized_batch = shared_json.normalize_batch_name(batch_id, allow_none=True)

    if explicit_issue_path:
        resolved = Path(explicit_issue_path).expanduser()
        if not resolved.is_absolute():
            resolved = resolved.resolve()
        resolved = _canonical_issue_path(resolved)
        if not resolved.exists():
            raise FileNotFoundError(f"issue artifact not found: {resolved}")
        return resolved

    if normalized_issue_id and normalized_batch:
        resolved = _resolve_issue_path_in_batch(issues_dir, normalized_batch, normalized_issue_id)
        if resolved is None:
            raise FileNotFoundError(
                f"issue record not found: {issues_dir / normalized_batch / f'issue_{normalized_issue_id}.json'}"
            )
        return resolved

    if normalized_issue_id:
        matches = _find_issue_paths_by_id(issues_dir, normalized_issue_id)
        matches.sort(key=issue_sort_key)
        if not matches:
            raise FileNotFoundError(f"issue_{normalized_issue_id}.json not found under {issues_dir}")
        if len(matches) > 1:
            joined = ", ".join(str(path.parent.name) for path in matches[:5])
            raise ValueError(
                f"issue_{normalized_issue_id}.json is ambiguous across batches: {joined}"
            )
        return matches[0]

    next_path = next_issue_path(issues_dir, batch_id=normalized_batch)
    if next_path is None:
        if normalized_batch:
            raise FileNotFoundError(f"no issue records found under {issues_dir / normalized_batch}")
        raise FileNotFoundError(f"no issue records found under {issues_dir}")
    return next_path


def issue_id(issue_path: Path) -> str:
    resolved_id = issue_id_from_artifact_name(issue_path.name)
    if resolved_id is None:
        return issue_path.stem
    return resolved_id


def load_issue_record(issue_path: Path) -> AnalyzeIssue:
    resolved_path = _canonical_issue_path(issue_path)
    if resolved_path.suffix.lower() == ".json":
        payload = shared_json.read_json_dict(resolved_path)
        if payload is None:
            raise ValueError(f"invalid issue record json: {resolved_path}")
        return issue_from_dict(payload)

    content = resolved_path.read_text(encoding="utf-8", errors="replace")
    return toon_issue_from_text(content, issue_path=resolved_path)


def issue_artifact_paths(issue_path: Path) -> list[Path]:
    canonical = _canonical_issue_path(issue_path)
    base_path = canonical.with_suffix("")
    artifacts: list[Path] = []
    for suffix in (".json", ".toon"):
        candidate = base_path.with_suffix(suffix)
        if candidate.exists():
            artifacts.append(candidate)
    return artifacts


def _canonical_issue_path(issue_path: Path) -> Path:
    suffix = issue_path.suffix.lower()
    if suffix == ".json" and issue_path.exists():
        return issue_path
    if suffix == ".toon":
        json_path = issue_path.with_suffix(".json")
        if json_path.exists():
            return json_path
    if suffix == ".json":
        return issue_path
    return issue_path


def _list_issue_record_paths(issues_dir: Path, batch_id: str | None = None) -> list[Path]:
    search_root = issues_dir / batch_id if batch_id else issues_dir
    if not search_root.exists():
        return []
    matches: dict[str, Path] = {}
    for suffix in (".json", ".toon"):
        for issue_path in search_root.rglob(f"issue_*{suffix}"):
            canonical = _canonical_issue_path(issue_path)
            matches[str(canonical)] = canonical
    return list(matches.values())


def _resolve_issue_path_in_batch(
    issues_dir: Path,
    batch_id: str,
    issue_id_value: str,
) -> Path | None:
    batch_dir = issues_dir / batch_id
    if not batch_dir.exists() or not batch_dir.is_dir():
        return None
    for suffix in (".json", ".toon"):
        candidate = batch_dir / f"issue_{issue_id_value}{suffix}"
        if candidate.exists():
            return _canonical_issue_path(candidate)
    return None


def _find_issue_paths_by_id(issues_dir: Path, issue_id_value: str) -> list[Path]:
    matches: list[Path] = []
    for suffix in (".json", ".toon"):
        matches.extend(issues_dir.rglob(f"issue_{issue_id_value}{suffix}"))
    normalized: dict[str, Path] = {}
    for match in matches:
        canonical = _canonical_issue_path(match)
        normalized[str(canonical)] = canonical
    return list(normalized.values())
