from __future__ import annotations

from pathlib import Path

CONFIG_PROFILES = ("distribution", "test")


def tracer_core_config_root(repo_root: Path, config_profile: str = "test") -> Path:
    normalized_profile = config_profile.strip().lower()
    if normalized_profile not in CONFIG_PROFILES:
        choices = ", ".join(CONFIG_PROFILES)
        raise ValueError(f"Unsupported config profile `{config_profile}`; expected one of: {choices}.")
    return repo_root / "assets" / "tracer_core" / f"config_{normalized_profile}"


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
        / "tracer_core"
        / "config"
    )
