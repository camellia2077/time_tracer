from __future__ import annotations

import tomllib
from pathlib import Path
from typing import Any

from .model import BundleModel


def validate_config_keys(config_data: dict[str, Any], target: str) -> None:
    defaults = config_data.get("defaults")
    if not isinstance(defaults, dict):
        raise ValueError("sync gate failed: config.toml missing [defaults] table.")
    for key in ("db_path", "output_root", "default_format"):
        if key not in defaults:
            raise ValueError(f"sync gate failed: config.toml missing defaults.{key}.")

    converter = config_data.get("converter")
    if not isinstance(converter, dict):
        raise ValueError("sync gate failed: config.toml missing [converter] table.")
    if "interval_config" not in converter:
        raise ValueError("sync gate failed: config.toml missing converter.interval_config.")

    reports = config_data.get("reports")
    if target == "windows":
        if not isinstance(reports, dict) or "markdown" not in reports:
            raise ValueError("sync gate failed: windows config must contain reports.markdown.")
    elif target == "android":
        if not isinstance(reports, dict) or "markdown" not in reports:
            raise ValueError("sync gate failed: android config must contain reports.markdown.")


def validate_sync_output(
    *,
    output_root: Path,
    planned_files: dict[str, bytes],
    model: BundleModel,
    target: str,
) -> None:
    for rel, expected in planned_files.items():
        path = output_root / rel
        if not path.is_file():
            raise ValueError(f"sync gate failed: missing output file: {path}")
        actual = path.read_bytes()
        if actual != expected:
            raise ValueError(f"sync gate failed: content mismatch after sync: {path}")

    bundle_path = output_root / "meta" / "bundle.toml"
    bundle_data = tomllib.loads(bundle_path.read_text(encoding="utf-8"))
    if int(bundle_data.get("schema_version", -1)) != model.schema_version:
        raise ValueError("sync gate failed: bundle.toml schema_version mismatch.")
    if str(bundle_data.get("profile", "")) != target:
        raise ValueError("sync gate failed: bundle.toml profile mismatch.")
    if str(bundle_data.get("bundle_name", "")) != model.bundle_name:
        raise ValueError("sync gate failed: bundle.toml bundle_name mismatch.")

    config_data = tomllib.loads((output_root / "config.toml").read_text(encoding="utf-8"))
    if not isinstance(config_data, dict):
        raise ValueError("sync gate failed: config.toml root is not a table.")
    validate_config_keys(config_data, target)
