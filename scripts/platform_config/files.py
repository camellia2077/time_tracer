from __future__ import annotations

from pathlib import Path

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


def apply_plan(
    output_root: Path, planned_files: dict[str, bytes], removed_files: list[str]
) -> None:
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
