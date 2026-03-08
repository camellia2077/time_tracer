import shutil

from ....core.context import Context
from .config_sync import (
    resolve_config_sync_target,
    resolve_platform_config_output_root,
    resolve_platform_config_source_root,
)
from .utils import normalize_cache_path


def resolve_windows_config_cmake_args(ctx: Context, app_name: str) -> list[str]:
    if resolve_config_sync_target(ctx, app_name) != "windows":
        return []
    output_root = resolve_platform_config_output_root(ctx, "windows")
    return ["-D", f"TRACER_WINDOWS_CONFIG_SOURCE_DIR={output_root}"]


def extract_cmake_definition_values(args: list[str], key: str) -> list[str]:
    values: list[str] = []
    prefixed = f"-D{key}="
    plain = f"{key}="
    for index, arg in enumerate(args):
        if arg.startswith(prefixed):
            values.append(arg[len(prefixed) :])
            continue
        if arg.startswith(plain):
            values.append(arg[len(plain) :])
            continue
        if arg == "-D" and index + 1 < len(args):
            next_arg = args[index + 1]
            if next_arg.startswith(plain):
                values.append(next_arg[len(plain) :])
    return values


def strip_cmake_definition(args: list[str], key: str) -> list[str]:
    stripped: list[str] = []
    plain = f"{key}="
    prefixed = f"-D{key}="
    skip_next = False
    for index, arg in enumerate(args):
        if skip_next:
            skip_next = False
            continue
        if arg.startswith(prefixed) or arg.startswith(plain):
            continue
        if arg == "-D" and index + 1 < len(args) and args[index + 1].startswith(plain):
            skip_next = True
            continue
        stripped.append(arg)
    return stripped


def validate_windows_config_source_override(
    ctx: Context,
    app_name: str,
    configure_args: list[str],
) -> tuple[bool, str]:
    if resolve_config_sync_target(ctx, app_name) != "windows":
        return True, ""

    key = "TRACER_WINDOWS_CONFIG_SOURCE_DIR"
    override_values = extract_cmake_definition_values(configure_args, key)
    if not override_values:
        return True, ""

    expected = normalize_cache_path(
        str(resolve_platform_config_output_root(ctx, "windows").resolve())
    )
    legacy_source = normalize_cache_path(str(resolve_platform_config_source_root(ctx).resolve()))
    for value in override_values:
        normalized = normalize_cache_path(value)
        if normalized == legacy_source:
            return (
                False,
                "Error: TRACER_WINDOWS_CONFIG_SOURCE_DIR cannot point to "
                f"source config root ({resolve_platform_config_source_root(ctx)}). "
                f"Use generated config root ({resolve_platform_config_output_root(ctx, 'windows')}).",
            )
        if normalized != expected:
            return (
                False,
                "Error: TRACER_WINDOWS_CONFIG_SOURCE_DIR must equal generated "
                f"windows config root ({resolve_platform_config_output_root(ctx, 'windows')}). "
                f"Received: {value}",
            )
    return True, ""


def sync_windows_runtime_config_copy_if_needed(
    ctx: Context,
    app_name: str,
    build_dir_name: str,
) -> int:
    if resolve_config_sync_target(ctx, app_name) != "windows":
        return 0

    build_dir = ctx.get_app_dir(app_name) / build_dir_name
    runtime_bin_dir = build_dir / "bin"
    if not runtime_bin_dir.exists():
        return 0

    source_root = resolve_platform_config_output_root(ctx, "windows")
    if not source_root.exists():
        print(f"Error: generated windows config root not found: {source_root}")
        return 1

    target_root = runtime_bin_dir / "config"
    try:
        if target_root.exists():
            shutil.rmtree(target_root)
        shutil.copytree(source_root, target_root)
    except OSError as exc:
        print(f"Error: failed to sync runtime config directory for `{app_name}`: {exc}")
        return 1
    return 0
