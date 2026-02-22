import sys
from collections.abc import Callable
from pathlib import Path

from ....core.context import Context
from ....core.executor import run_command


def resolve_platform_config_source_root(ctx: Context) -> Path:
    return ctx.repo_root / "apps" / "time_tracer" / "config"


def resolve_platform_config_output_root(ctx: Context, sync_target: str) -> Path:
    if sync_target == "windows":
        return ctx.repo_root / "apps" / "tracer_windows_cli" / "config"
    if sync_target == "android":
        return (
            ctx.repo_root
            / "apps"
            / "tracer_android"
            / "runtime"
            / "src"
            / "main"
            / "assets"
            / "time_tracer"
            / "config"
        )
    raise ValueError(f"Unsupported config sync target: {sync_target}")


def resolve_platform_config_runner_path(ctx: Context) -> Path | None:
    script_path = ctx.repo_root / "scripts" / "platform_config" / "run.py"
    if script_path.exists():
        return script_path
    return None


def resolve_config_sync_target(ctx: Context, app_name: str) -> str | None:
    app = ctx.get_app_metadata(app_name)
    raw_target = (getattr(app, "config_sync_target", "") or "").strip().lower()
    if not raw_target:
        return None
    if raw_target in {"windows", "android"}:
        return raw_target
    print(
        f"Warning: unknown config_sync_target `{raw_target}` for app "
        f"`{app_name}`; skipping config sync."
    )
    return None


def sync_platform_config_if_needed(
    ctx: Context,
    app_name: str,
    run_command_fn: Callable[..., int] | None = None,
) -> int:
    effective_run_command = run_command if run_command_fn is None else run_command_fn
    sync_target = resolve_config_sync_target(ctx, app_name)
    if sync_target is None:
        return 0

    sync_script_path = resolve_platform_config_runner_path(ctx)
    if sync_script_path is None:
        print(
            "Error: platform config sync script not found: "
            f"{ctx.repo_root / 'scripts' / 'platform_config' / 'run.py'}"
        )
        return 1

    source_root = resolve_platform_config_source_root(ctx)
    output_root = resolve_platform_config_output_root(ctx, sync_target)
    sync_cmd = [
        sys.executable,
        str(sync_script_path),
        "--target",
        sync_target,
        "--source-root",
        str(source_root),
        "--apply",
    ]
    if sync_target == "windows":
        sync_cmd += ["--windows-output-root", str(output_root)]
    elif sync_target == "android":
        sync_cmd += ["--android-output-root", str(output_root)]

    print(f"--- build: auto-sync platform config before build (target={sync_target}).")
    return effective_run_command(
        sync_cmd,
        cwd=ctx.repo_root,
        env=ctx.setup_env(),
    )
