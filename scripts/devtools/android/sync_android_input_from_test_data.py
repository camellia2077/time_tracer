#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import shutil
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Sync Android runtime input/full from canonical test/data."
    )
    parser.add_argument(
        "--source-root",
        default="test/data",
        help="Canonical source root (default: test/data).",
    )
    parser.add_argument(
        "--android-input-full-root",
        default="apps/tracer_android/runtime/src/main/assets/tracer_core/input/full",
        help="Android assets input/full root.",
    )
    parser.add_argument(
        "--apply",
        action="store_true",
        help="Apply sync changes.",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Fail when drift exists (read-only).",
    )
    parser.add_argument(
        "--prune-managed-years",
        action="store_true",
        help="Delete target files under managed years that are absent in source.",
    )
    return parser.parse_args()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as fp:
        for chunk in iter(lambda: fp.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def collect_txt_files(root: Path) -> dict[str, Path]:
    files: dict[str, Path] = {}
    for file_path in sorted(root.rglob("*.txt")):
        rel = file_path.relative_to(root).as_posix()
        files[rel] = file_path
    return files


def main() -> int:
    args = parse_args()
    if not args.apply and not args.check:
        raise SystemExit("One mode is required: use --apply or --check.")

    source_root = Path(args.source_root).resolve()
    target_root = Path(args.android_input_full_root).resolve()

    if not source_root.exists():
        raise SystemExit(f"Missing source root: {source_root}")
    target_root.mkdir(parents=True, exist_ok=True)

    source_files = collect_txt_files(source_root)
    target_files = collect_txt_files(target_root)

    to_copy: list[str] = []
    for rel, src_path in source_files.items():
        dst_path = target_root / rel
        if not dst_path.exists():
            to_copy.append(rel)
            continue
        if sha256(src_path) != sha256(dst_path):
            to_copy.append(rel)

    managed_years = {Path(rel).parts[0] for rel in source_files if "/" in rel}
    to_remove: list[str] = []
    if args.prune_managed_years:
        for rel in target_files:
            rel_parts = Path(rel).parts
            if not rel_parts:
                continue
            if rel_parts[0] in managed_years and rel not in source_files:
                to_remove.append(rel)

    if not to_copy and not to_remove:
        print("--- sync input: no drift")
        return 0

    print(f"--- sync input: drift detected copy={len(to_copy)} remove={len(to_remove)}")
    for rel in to_copy[:20]:
        print(f"  COPY {rel}")
    for rel in to_remove[:20]:
        print(f"  REMOVE {rel}")

    if args.check:
        return 1

    for rel in to_copy:
        src = source_root / rel
        dst = target_root / rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)
    for rel in to_remove:
        dst = target_root / rel
        if dst.exists():
            dst.unlink()

    print("--- sync input: applied")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
