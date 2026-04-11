from __future__ import annotations

import json
import shutil
from pathlib import Path

try:
    import tomllib
except ImportError:  # pragma: no cover
    import tomli as tomllib  # type: ignore

from tools.toolchain.core.generated_paths import resolve_build_layout

DEFAULT_APP_NAME = "tracer_windows_rust_cli"
DEFAULT_CLI_EXE = "time_tracer_cli.exe"
DEFAULT_RUNTIME_FILES = [
    "time_tracer_cli.exe",
    "tracer_core.dll",
    "reports_shared.dll",
    "libsqlite3-0.dll",
    "libtomlplusplus-3.dll",
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll",
]
DEFAULT_RUNTIME_FOLDERS = ["config"]
DEFAULT_CORE_DLL = "tracer_core.dll"
DEFAULT_REPORTS_SHARED_DLL = "reports_shared.dll"


def auto_detect_build_dir(repo_root: Path, app_name: str) -> str | None:
    for candidate in ("build_fast", "build_agent", "build_tidy", "build"):
        if resolve_build_layout(repo_root, app_name, candidate).bin_dir.is_dir():
            return candidate
    return None


def load_runtime_bundle_spec(repo_root: Path) -> tuple[list[str], list[str]]:
    env_path = repo_root / "tools" / "suites" / "tracer_windows_rust_cli" / "env.toml"
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


def _load_runtime_manifest(source_bin: Path) -> tuple[list[str], list[str]] | None:
    manifest_path = source_bin / "runtime_manifest.json"
    if not manifest_path.is_file():
        return None
    try:
        payload = json.loads(manifest_path.read_text(encoding="utf-8"))
    except Exception:
        return None
    if not isinstance(payload, dict):
        return None
    runtime = payload.get("runtime")
    if not isinstance(runtime, dict):
        return None
    raw_files = runtime.get("required_files")
    raw_dirs = runtime.get("required_dirs")
    files = [item for item in (raw_files or []) if isinstance(item, str) and item]
    folders = [item for item in (raw_dirs or []) if isinstance(item, str) and item]
    if not files:
        return None
    return files, folders


def resolve_runtime_library_names(source_bin: Path) -> tuple[str, str]:
    manifest_path = source_bin / "runtime_manifest.json"
    if not manifest_path.is_file():
        return DEFAULT_CORE_DLL, DEFAULT_REPORTS_SHARED_DLL
    try:
        payload = json.loads(manifest_path.read_text(encoding="utf-8"))
    except Exception:
        return DEFAULT_CORE_DLL, DEFAULT_REPORTS_SHARED_DLL
    if not isinstance(payload, dict):
        return DEFAULT_CORE_DLL, DEFAULT_REPORTS_SHARED_DLL
    libraries = payload.get("libraries")
    if not isinstance(libraries, dict):
        return DEFAULT_CORE_DLL, DEFAULT_REPORTS_SHARED_DLL
    core_dll = libraries.get("core")
    reports_shared_dll = libraries.get("reports_shared")
    if not isinstance(core_dll, str) or not core_dll:
        core_dll = DEFAULT_CORE_DLL
    if not isinstance(reports_shared_dll, str) or not reports_shared_dll:
        reports_shared_dll = DEFAULT_REPORTS_SHARED_DLL
    return core_dll, reports_shared_dll


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
    manifest_spec = _load_runtime_manifest(source_bin)
    required = [source_bin / DEFAULT_CLI_EXE]
    if manifest_spec is not None:
        required_files, required_folders = manifest_spec
        required.extend(source_bin / rel for rel in required_files)
        required.extend(source_bin / rel for rel in required_folders)
    else:
        required.extend(
            [
                source_bin / DEFAULT_CORE_DLL,
                source_bin / DEFAULT_REPORTS_SHARED_DLL,
                source_bin / "config" / "config.toml",
            ]
        )
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
    return resolve_build_layout(repo_root, DEFAULT_APP_NAME, build_dir).bin_dir
