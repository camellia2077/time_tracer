from __future__ import annotations

import hashlib
import json
from datetime import UTC, datetime
from pathlib import Path
from typing import Any


def state_path(output_root: Path) -> Path:
    return output_root / "meta" / "sync_state.json"


def compute_input_hash(target: str, source_root: Path, file_hashes: dict[str, str]) -> str:
    hasher = hashlib.sha256()
    hasher.update(target.encode("utf-8"))
    hasher.update(b"\0")
    hasher.update(str(source_root.resolve()).encode("utf-8"))
    hasher.update(b"\0")
    for rel, digest in sorted(file_hashes.items()):
        hasher.update(rel.encode("utf-8"))
        hasher.update(b"\0")
        hasher.update(digest.encode("utf-8"))
        hasher.update(b"\0")
    return hasher.hexdigest()


def load_state(output_root: Path) -> dict[str, Any] | None:
    path = state_path(output_root)
    if not path.exists():
        return None
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None
    if not isinstance(data, dict):
        return None
    return data


def write_state(
    *,
    output_root: Path,
    target: str,
    source_root: Path,
    input_hash: str,
    file_hashes: dict[str, str],
    planned_file_count: int,
) -> None:
    payload: dict[str, Any] = {
        "target": target,
        "source_root": str(source_root.resolve()),
        "input_hash": input_hash,
        "planned_file_count": planned_file_count,
        "file_hashes": file_hashes,
        "generated_at_utc": datetime.now(UTC).isoformat(),
    }
    path = state_path(output_root)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def is_cache_hit(
    *,
    output_root: Path,
    expected_target: str,
    expected_source_root: Path,
    expected_input_hash: str,
    expected_file_hashes: dict[str, str],
    hash_file_fn,
) -> bool:
    state = load_state(output_root)
    if not state:
        return False

    if state.get("target") != expected_target:
        return False
    if state.get("source_root") != str(expected_source_root.resolve()):
        return False
    if state.get("input_hash") != expected_input_hash:
        return False

    state_hashes = state.get("file_hashes")
    if not isinstance(state_hashes, dict):
        return False
    if state_hashes != expected_file_hashes:
        return False

    for rel, digest in expected_file_hashes.items():
        target_path = output_root / rel
        if not target_path.is_file():
            return False
        if hash_file_fn(target_path) != digest:
            return False
    return True
