from pathlib import Path

from ...core.context import Context


def resolve_profile(default_profile: str | None, app_name: str) -> str:
    if default_profile:
        return default_profile
    return "android" if app_name == "tracer_android" else "windows"


def resolve_config_root(ctx: Context, app_name: str, config_root_arg: str | None) -> Path:
    if config_root_arg:
        candidate = Path(config_root_arg)
        return (candidate if candidate.is_absolute() else (ctx.repo_root / candidate)).resolve()

    if app_name in {"tracer_core", "tracer_windows_cli"}:
        return (ctx.repo_root / "apps" / "tracer_core" / "config").resolve()
    if app_name == "tracer_android":
        return (
            ctx.repo_root
            / "apps"
            / "tracer_android"
            / "runtime"
            / "src"
            / "main"
            / "assets"
            / "tracer_core"
            / "config"
        ).resolve()

    fallback = (ctx.get_app_dir(app_name) / "config").resolve()
    if fallback.exists():
        return fallback
    return (ctx.repo_root / "apps" / "tracer_core" / "config").resolve()


def to_absolute_path(config_root: Path, value: str) -> Path:
    path = Path(value)
    if path.is_absolute():
        return path
    return (config_root / path).resolve()


def to_bundle_relative_path(config_root: Path, absolute_path: Path) -> str:
    resolved = absolute_path.resolve()
    try:
        return resolved.relative_to(config_root).as_posix()
    except ValueError:
        return resolved.as_posix()
