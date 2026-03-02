from __future__ import annotations

import json
from pathlib import Path


def append_unique_reasons(target: list[str], extra: list[str]) -> None:
    for reason in extra:
        if reason not in target:
            target.append(reason)


def filter_auto_reasons(
    reasons: list[str],
    *,
    allow_no_such_file: bool,
    allow_glob_mismatch: bool,
) -> list[str]:
    filtered: list[str] = []
    for reason in reasons:
        if reason == "auto_no_such_file" and not allow_no_such_file:
            continue
        if reason == "auto_glob_mismatch" and not allow_glob_mismatch:
            continue
        filtered.append(reason)
    return filtered


def rename_report_mtime_ns(build_dir: Path) -> int | None:
    report_path = build_dir / "rename" / "rename_apply_report.json"
    if not report_path.exists():
        return None
    try:
        return report_path.stat().st_mtime_ns
    except OSError:
        return None


def high_already_renamed_summary(
    build_dir: Path,
    *,
    ratio_threshold: float,
    min_count: int,
) -> str | None:
    report_path = build_dir / "rename" / "rename_apply_report.json"
    if not report_path.exists():
        return None
    try:
        payload = json.loads(report_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None
    results = payload.get("results", [])
    if not isinstance(results, list):
        return None

    total = 0
    already_renamed = 0
    for item in results:
        if not isinstance(item, dict):
            continue
        total += 1
        if item.get("reason") == "already_renamed":
            already_renamed += 1
    if total <= 0:
        return None
    ratio = already_renamed / total
    if already_renamed < max(1, min_count):
        return None
    if ratio < ratio_threshold:
        return None
    return f"{already_renamed}/{total} ({ratio:.1%})"
