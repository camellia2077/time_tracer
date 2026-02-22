from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class BundleModel:
    schema_version: int
    profile: str
    bundle_name: str
    required_files: list[str]
    optional_files: list[str]
    converter_interval_config: str
    reports: dict[str, dict[str, str]]
