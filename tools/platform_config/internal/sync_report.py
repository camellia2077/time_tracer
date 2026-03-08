from __future__ import annotations

import difflib
import json
from pathlib import Path


def print_bundle_diff(output_root: Path, generated_bundle: str) -> None:
    bundle_path = output_root / "meta" / "bundle.toml"
    existing_text = bundle_path.read_text(encoding="utf-8") if bundle_path.exists() else ""
    diff_lines = list(
        difflib.unified_diff(
            existing_text.splitlines(),
            generated_bundle.splitlines(),
            fromfile=f"{bundle_path} (current)",
            tofile=f"{bundle_path} (generated)",
            lineterm="",
        )
    )
    if diff_lines:
        print("\n".join(diff_lines))
    else:
        print("--- bundle.toml is already up to date.")


def print_sync_header(
    *,
    target: str,
    source_root: Path,
    output_root: Path,
    planned_files: int,
    added: int,
    changed: int,
    removed: int,
) -> None:
    print(f"=== target: {target}")
    print(f"source: {source_root}")
    print(f"output: {output_root}")
    print(f"planned files: {planned_files}")
    print(f"changes: +{added} ~{changed} -{removed}")


def print_sync_report(
    *,
    target: str,
    source_root: Path,
    output_root: Path,
    planned_files: int,
    added: int,
    changed: int,
    removed: int,
    cache_hit: bool,
    applied: bool,
    duration_ms: int,
) -> None:
    print(
        "sync_report="
        + json.dumps(
            {
                "target": target,
                "source": str(source_root),
                "output": str(output_root),
                "planned_files": planned_files,
                "added": added,
                "changed": changed,
                "removed": removed,
                "cache_hit": cache_hit,
                "applied": applied,
                "duration_ms": duration_ms,
            },
            ensure_ascii=False,
            sort_keys=True,
        )
    )
