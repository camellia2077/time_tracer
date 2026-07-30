from __future__ import annotations

from pathlib import Path

CONFIG_PROFILES = ("distribution", "test")


def tracer_core_config_root(repo_root: Path, config_profile: str = "test") -> Path:
    normalized_profile = config_profile.strip().lower()
    if normalized_profile not in CONFIG_PROFILES:
        choices = ", ".join(CONFIG_PROFILES)
        raise ValueError(f"Unsupported config profile `{config_profile}`; expected one of: {choices}.")
    # Static program resources are shared by both profiles. The profile now
    # selects only the mutable activity-hierarchy source.
    return repo_root / "config" / "program"


def tracer_core_activity_hierarchy_root(repo_root: Path, config_profile: str = "test") -> Path:
    normalized_profile = config_profile.strip().lower()
    if normalized_profile not in CONFIG_PROFILES:
        choices = ", ".join(CONFIG_PROFILES)
        raise ValueError(f"Unsupported config profile `{config_profile}`; expected one of: {choices}.")
    if normalized_profile == "distribution":
        return repo_root / "config" / "user"
    return repo_root / "test" / "data" / "activity_hierarchy"


def windows_cli_config_root(repo_root: Path) -> Path:
    return repo_root / "apps" / "cli" / "windows" / "rust" / "runtime" / "config"


def windows_cli_assets_root(repo_root: Path) -> Path:
    return repo_root / "apps" / "cli" / "windows" / "rust" / "runtime" / "assets"


def android_config_root(repo_root: Path) -> Path:
    return (
        repo_root
        / "apps"
        / "android"
        / "runtime"
        / "src"
        / "main"
        / "assets"
        / "config"
    )
