from __future__ import annotations

from pathlib import Path


def tracer_core_config_root(repo_root: Path) -> Path:
    return repo_root / "apps" / "tracer_core" / "config"


def windows_cli_config_root(repo_root: Path) -> Path:
    return repo_root / "apps" / "tracer_cli" / "windows" / "rust_cli" / "runtime" / "config"


def windows_cli_assets_root(repo_root: Path) -> Path:
    return repo_root / "apps" / "tracer_cli" / "windows" / "rust_cli" / "runtime" / "assets"


def android_config_root(repo_root: Path) -> Path:
    return (
        repo_root
        / "apps"
        / "tracer_android"
        / "runtime"
        / "src"
        / "main"
        / "assets"
        / "tracer_core"
        / "config"
    )
