import os
import shutil
import json
from collections.abc import Callable
from pathlib import Path

from tools.platform_paths import windows_cli_assets_root, windows_cli_config_root

from ...core.context import Context
from ...core.executor import run_command
from . import common as build_common
from .cargo_locator import resolve_cargo_executable
from .windows_icon_resources import prepare_windows_cli_icon_ico, should_enable_windows_cli_icon


def _resolve_profile_flag(profile_name: str | None) -> list[str]:
    name = (profile_name or "").strip().lower()
    if not name:
        return []
    if "release" in name:
        return ["--release"]
    return []


def _filter_passthrough_args(args: list[str] | None) -> list[str]:
    return [item for item in (args or []) if item != "--"]


def _artifact_name() -> str:
    return "time_tracer_cli.exe" if os.name == "nt" else "time_tracer_cli"


def _profile_output_dir(profile_name: str | None) -> str:
    return "release" if "release" in (profile_name or "").strip().lower() else "debug"


def configure_cargo(
    app_name: str,
    tidy: bool,
    extra_args: list[str] | None,
    cmake_args: list[str] | None,
    build_dir_name: str | None,
) -> int:
    if tidy:
        print("--- configure: cargo backend does not use `--tidy`; flag ignored.")
    if build_dir_name:
        print(f"--- configure: cargo backend will use build-dir ({build_dir_name}) at build stage.")
    filtered_extra_args = [a for a in (extra_args or []) if a != "--"]
    filtered_cmake_args = [a for a in (cmake_args or []) if a != "--"]
    if filtered_extra_args or filtered_cmake_args:
        print("--- configure: cargo backend does not accept configure args; extra args ignored.")
    print("--- configure: app uses cargo backend; configure stage skipped.")
    return 0


def _copy_cli_artifact(app_dir: Path, build_dir_name: str, profile_name: str | None) -> int:
    artifact = _artifact_name()
    source_path = (
        app_dir / build_dir_name / "cargo_target" / _profile_output_dir(profile_name) / artifact
    )
    target_dir = app_dir / build_dir_name / "bin"
    target_path = target_dir / artifact

    if not source_path.exists():
        print(f"Error: cargo output executable not found: {source_path}")
        return 1

    target_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source_path, target_path)
    print(f"--- build: copied cargo artifact to {target_path}")
    return 0


def _copy_file_if_present(source: Path, target_dir: Path) -> bool:
    if not source.exists():
        return False
    target_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, target_dir / source.name)
    return True


def _copy_dir_if_present(source: Path, target: Path) -> bool:
    if not source.exists():
        return False
    if target.exists():
        shutil.rmtree(target)
    shutil.copytree(source, target)
    return True


def _first_existing_path(candidates: list[Path]) -> Path | None:
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return None


def _load_runtime_manifest_required_files(manifest_path: Path) -> list[str]:
    try:
        payload = json.loads(manifest_path.read_text(encoding="utf-8"))
    except Exception:
        return []
    if not isinstance(payload, dict):
        return []
    runtime = payload.get("runtime")
    if not isinstance(runtime, dict):
        return []
    files = runtime.get("required_files")
    if not isinstance(files, list):
        return []
    selected: list[str] = []
    for item in files:
        if isinstance(item, str) and item.strip():
            selected.append(item.strip())
    return selected


def _env_truthy(name: str) -> bool:
    value = (os.getenv(name) or "").strip().lower()
    return value in {"1", "true", "yes", "on"}


def _sync_windows_runtime_layout_for_rust(
    ctx: Context,
    app_name: str,
    build_dir_name: str,
) -> int:
    if os.name != "nt":
        return 0
    if app_name != "tracer_windows_rust_cli":
        return 0

    app_dir = ctx.get_app_dir(app_name)
    runtime_bin_dir = app_dir / build_dir_name / "bin"
    runtime_bin_dir.mkdir(parents=True, exist_ok=True)

    core_dir = ctx.get_app_dir("tracer_core")

    runtime_bin_candidates = [
        core_dir / build_dir_name / "bin",
    ]
    # When strict sync is enabled, only probe the current build dir.
    # This is useful for intentionally verifying warning paths where
    # core DLLs are absent for Rust-only builds.
    if not _env_truthy("TT_RUST_RUNTIME_SYNC_STRICT"):
        runtime_bin_candidates.extend(
            [
                core_dir / "build_fast" / "bin",
                core_dir / "build" / "bin",
            ]
        )

    copied_files: list[str] = []

    manifest_source = _first_existing_path(
        [candidate / "runtime_manifest.json" for candidate in runtime_bin_candidates]
    )
    required_files = ["tracer_core.dll", "reports_shared.dll"]
    if manifest_source:
        loaded = _load_runtime_manifest_required_files(manifest_source)
        if loaded:
            # Only sync root-level DLL files from core runtime output.
            required_files = [
                item
                for item in loaded
                if "/" not in item and "\\" not in item and item.lower().endswith(".dll")
            ]
    for runtime_file in required_files:
        source_file = _first_existing_path(
            [candidate / runtime_file for candidate in runtime_bin_candidates]
        )
        if source_file and _copy_file_if_present(source_file, runtime_bin_dir):
            copied_files.append(runtime_file)
            continue
        print(
            f"Warning: runtime dependency `{runtime_file}` not found for Rust CLI runtime layout. "
            "Build output may not be directly runnable."
        )

    if manifest_source and _copy_file_if_present(manifest_source, runtime_bin_dir):
        copied_files.append("runtime_manifest.json")

    config_source = windows_cli_config_root(ctx.repo_root)
    if _copy_dir_if_present(config_source, runtime_bin_dir / "config"):
        copied_files.append("config/")
    else:
        print(
            "Warning: generated windows config root not found for Rust CLI runtime layout: "
            f"{config_source}"
        )

    assets_source = windows_cli_assets_root(ctx.repo_root)
    if _copy_dir_if_present(assets_source, runtime_bin_dir / "assets"):
        copied_files.append("assets/")

    if copied_files:
        print("--- build: synchronized Rust runtime layout files: " + ", ".join(copied_files))
    return 0


def _build_cargo_args(ctx: Context, profile_name: str | None) -> list[str]:
    cargo_args: list[str] = []
    cargo_args.extend(_resolve_profile_flag(profile_name))
    for profile_arg in build_common.profile_cargo_args(ctx, profile_name):
        if profile_arg not in cargo_args:
            cargo_args.append(profile_arg)
    return cargo_args


def _build_cargo_environment(ctx: Context, app_dir: Path, build_dir_name: str) -> dict[str, str]:
    env = ctx.setup_env()
    env["CARGO_TARGET_DIR"] = str((app_dir / build_dir_name / "cargo_target").resolve())
    return env


def _build_cargo_command(
    cargo_bin: str,
    cargo_args: list[str],
    passthrough_args: list[str],
) -> list[str]:
    return [cargo_bin, "build", *cargo_args, *passthrough_args]


def build_cargo(
    ctx: Context,
    app_name: str,
    tidy: bool,
    extra_args: list[str] | None,
    cmake_args: list[str] | None,
    build_dir_name: str | None,
    profile_name: str | None,
    windows_icon_svg: str | None = None,
    run_command_fn: Callable[..., int] | None = None,
) -> int:
    effective_run_command = run_command if run_command_fn is None else run_command_fn
    app_dir = ctx.get_app_dir(app_name)
    resolved_build_dir_name = (build_dir_name or "").strip() or "build_fast"

    if tidy:
        print("--- build: cargo backend does not use `--tidy`; flag ignored.")
    filtered_cmake_args = _filter_passthrough_args(cmake_args)
    if filtered_cmake_args:
        print("--- build: cargo backend ignores --cmake-args.")

    cargo_args = _build_cargo_args(ctx, profile_name)
    passthrough_args = _filter_passthrough_args(extra_args)
    env = _build_cargo_environment(ctx, app_dir, resolved_build_dir_name)
    icon_ico = prepare_windows_cli_icon_ico(
        ctx=ctx,
        app_name=app_name,
        app_dir=app_dir,
        build_dir_name=resolved_build_dir_name,
        profile_name=profile_name,
        svg_override=windows_icon_svg,
    )
    if should_enable_windows_cli_icon(app_name=app_name, profile_name=profile_name):
        if icon_ico is None:
            return 1
        env["TT_WINDOWS_CLI_ICON_ICO"] = str(icon_ico)

    cargo_bin = resolve_cargo_executable(env)
    cargo_cmd = _build_cargo_command(
        cargo_bin=cargo_bin,
        cargo_args=cargo_args,
        passthrough_args=passthrough_args,
    )
    build_ret = effective_run_command(
        cargo_cmd,
        cwd=app_dir,
        env=env,
    )
    if build_ret != 0:
        return build_ret
    copy_ret = _copy_cli_artifact(
        app_dir=app_dir,
        build_dir_name=resolved_build_dir_name,
        profile_name=profile_name,
    )
    if copy_ret != 0:
        return copy_ret
    return _sync_windows_runtime_layout_for_rust(
        ctx=ctx,
        app_name=app_name,
        build_dir_name=resolved_build_dir_name,
    )
