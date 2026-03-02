from __future__ import annotations

import hashlib
import shutil
from pathlib import Path
from uuid import uuid4

from .bundle import build_android_config_toml, render_bundle_toml
from .model import BundleModel
from .path_utils import dedupe_keep_order


def collect_plan_files(source_root: Path, model: BundleModel) -> dict[str, bytes]:
    files_to_copy = dedupe_keep_order([*model.required_files, *model.optional_files])
    plan: dict[str, bytes] = {}
    for rel in files_to_copy:
        source_path = source_root / rel
        if not source_path.exists() or not source_path.is_file():
            raise FileNotFoundError(f"Required source file missing: {source_path}")
        plan[rel] = source_path.read_bytes()

    bundle_bytes = render_bundle_toml(model).encode("utf-8")
    if model.profile == "android":
        plan["config.toml"] = build_android_config_toml(source_root).encode("utf-8")
    plan["meta/bundle.toml"] = bundle_bytes
    return plan


def read_existing_files(root: Path) -> dict[str, bytes]:
    if not root.exists():
        return {}
    existing: dict[str, bytes] = {}
    for file_path in root.rglob("*"):
        if not file_path.is_file():
            continue
        rel = file_path.relative_to(root).as_posix()
        existing[rel] = file_path.read_bytes()
    return existing


def hash_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def build_file_hashes(planned_files: dict[str, bytes]) -> dict[str, str]:
    return {rel: hash_bytes(data) for rel, data in sorted(planned_files.items())}


def _write_plan_tree(root: Path, planned_files: dict[str, bytes]) -> None:
    root.mkdir(parents=True, exist_ok=True)
    for rel, data in planned_files.items():
        target = root / rel
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)


def apply_plan(
    output_root: Path, planned_files: dict[str, bytes], removed_files: list[str]
) -> None:
    # Keep legacy in-place writer for dry-run utilities that may still depend on
    # this behavior.
    output_root.mkdir(parents=True, exist_ok=True)

    for rel in removed_files:
        target = output_root / rel
        if target.exists():
            target.unlink()

    for rel, data in planned_files.items():
        target = output_root / rel
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)

    for directory in sorted((p for p in output_root.rglob("*") if p.is_dir()), reverse=True):
        try:
            directory.rmdir()
        except OSError:
            continue


def apply_plan_atomic(output_root: Path, planned_files: dict[str, bytes]) -> None:
    parent = output_root.parent
    parent.mkdir(parents=True, exist_ok=True)

    stage_dir = parent / f".{output_root.name}.stage.{uuid4().hex}"
    backup_dir = parent / f".{output_root.name}.backup.{uuid4().hex}"

    _write_plan_tree(stage_dir, planned_files)

    try:
        if output_root.exists():
            output_root.rename(backup_dir)
        stage_dir.rename(output_root)
        if backup_dir.exists():
            shutil.rmtree(backup_dir)
    except Exception:
        if output_root.exists():
            shutil.rmtree(output_root, ignore_errors=True)
        if backup_dir.exists():
            backup_dir.rename(output_root)
        raise
    finally:
        if stage_dir.exists():
            shutil.rmtree(stage_dir, ignore_errors=True)
