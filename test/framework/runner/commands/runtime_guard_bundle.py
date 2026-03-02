from __future__ import annotations

import shutil
from pathlib import Path

try:
    import tomllib
except ImportError:  # pragma: no cover
    import tomli as tomllib  # type: ignore

from .run_support import resolve_app_root

DEFAULT_APP_NAME = "tracer_windows_rust_cli"
DEFAULT_CLI_EXE = "time_tracer_cli.exe"
DEFAULT_RUNTIME_FILES = [
    "time_tracer_cli.exe",
    "tracer_core.dll",
    "libreports_shared.dll",
    "libsqlite3-0.dll",
    "libtomlplusplus-3.dll",
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll",
]
DEFAULT_RUNTIME_FOLDERS = ["config"]


def auto_detect_build_dir(repo_root: Path, app_name: str) -> str | None:
    app_root = resolve_app_root(repo_root, app_name)
    for candidate in ("build_fast", "build_agent", "build_tidy", "build"):
        if (app_root / candidate / "bin").is_dir():
            return candidate
    return None


def load_runtime_bundle_spec(repo_root: Path) -> tuple[list[str], list[str]]:
    env_path = repo_root / "test" / "suites" / "tracer_windows_rust_cli" / "env.toml"
    if not env_path.exists():
        return DEFAULT_RUNTIME_FILES, DEFAULT_RUNTIME_FOLDERS

    try:
        with env_path.open("rb") as file:
            payload = tomllib.load(file)
    except Exception:
        return DEFAULT_RUNTIME_FILES, DEFAULT_RUNTIME_FOLDERS

    cleanup = payload.get("cleanup", {})
    if not isinstance(cleanup, dict):
        return DEFAULT_RUNTIME_FILES, DEFAULT_RUNTIME_FOLDERS

    files = cleanup.get("files_to_copy")
    folders = cleanup.get("folders_to_copy")

    selected_files = (
        [item for item in files if isinstance(item, str)] if isinstance(files, list) else []
    )
    selected_folders = (
        [item for item in folders if isinstance(item, str)] if isinstance(folders, list) else []
    )

    if not selected_files:
        selected_files = list(DEFAULT_RUNTIME_FILES)
    if not selected_folders:
        selected_folders = list(DEFAULT_RUNTIME_FOLDERS)
    return selected_files, selected_folders


def copy_runtime_bundle(
    source_bin: Path,
    dest_bin: Path,
    files_to_copy: list[str],
    folders_to_copy: list[str],
) -> None:
    _ = files_to_copy, folders_to_copy
    # Runtime-guard now validates against full runtime layout to avoid hidden
    # dependency drift between selective-copy fixtures and actual release bundles.
    shutil.copytree(source_bin, dest_bin, dirs_exist_ok=True)


def ensure_source_runtime_ready(source_bin: Path) -> None:
    required = [
        source_bin / DEFAULT_CLI_EXE,
        source_bin / "tracer_core.dll",
        source_bin / "libreports_shared.dll",
        source_bin / "config" / "config.toml",
    ]
    missing = [str(path) for path in required if not path.exists()]
    if missing:
        raise RuntimeError(
            "Runtime guard source bundle is incomplete. Missing:\n- " + "\n- ".join(missing)
        )


def resolve_source_bin_dir(repo_root: Path, args) -> Path:
    if args.bin_dir:
        return Path(args.bin_dir).resolve()

    build_dir = args.build_dir
    if not build_dir:
        build_dir = auto_detect_build_dir(repo_root, DEFAULT_APP_NAME)
    if not build_dir:
        raise RuntimeError("No build dir detected. Use --build-dir, --bin-dir, or build first.")
    app_root = resolve_app_root(repo_root, DEFAULT_APP_NAME)
    return (app_root / build_dir / "bin").resolve()
