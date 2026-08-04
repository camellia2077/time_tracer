from __future__ import annotations

import hashlib
from pathlib import Path

from .task_record_types import SourceFingerprint


def compute_source_fingerprint(path_like: str | Path | None) -> SourceFingerprint | None:
    raw_text = str(path_like or "").strip()
    if not raw_text:
        return None

    source_path = Path(raw_text)
    try:
        source_path = source_path.resolve()
    except OSError:
        return None
    if not source_path.exists() or not source_path.is_file():
        return None

    try:
        payload = source_path.read_bytes()
        stat_result = source_path.stat()
    except OSError:
        return None

    return SourceFingerprint(
        mtime_ns=int(stat_result.st_mtime_ns),
        size_bytes=int(stat_result.st_size),
        sha256=hashlib.sha256(payload).hexdigest(),
    )


def fingerprints_match(
    left: SourceFingerprint | None,
    right: SourceFingerprint | None,
) -> bool:
    if left is None or right is None:
        return True
    return (
        left.mtime_ns == right.mtime_ns
        and left.size_bytes == right.size_bytes
        and left.sha256 == right.sha256
    )
