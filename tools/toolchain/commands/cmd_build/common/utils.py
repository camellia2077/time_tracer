from pathlib import Path

from ....core.context import Context


def normalize_cache_path(path_value: str) -> str:
    return path_value.strip().replace("\\", "/").rstrip("/")


def to_absolute_path(path_str: str) -> Path:
    return Path(path_str).resolve()


def resolve_command_output_log_path(
    *,
    ctx: Context,
    app_name: str,
    backend: str,
    build_dir_name: str,
    tidy: bool = False,
) -> Path:
    if backend == "gradle":
        return (ctx.get_app_dir(app_name) / "build" / "build.log").resolve()
    if tidy:
        return (ctx.get_tidy_dir(app_name, build_dir_name) / "build.log").resolve()
    return (ctx.get_build_dir(app_name, build_dir_name) / "build.log").resolve()
