from ....core.context import Context
from .config_sync import (
    resolve_config_sync_target,
    resolve_platform_config_output_root,
    resolve_platform_config_source_root,
)
from .utils import normalize_cache_path


def resolve_android_config_gradle_args(extra_args: list[str]) -> bool:
    return any(arg.startswith("-PtimeTracerConfigRoot=") for arg in extra_args)


def resolve_android_native_optimization_gradle_args(extra_args: list[str]) -> bool:
    return any(arg.startswith("-PtimeTracerDisableNativeOptimization=") for arg in extra_args)


def extract_gradle_property_values(args: list[str], key: str) -> list[str]:
    prefix = f"-P{key}="
    values: list[str] = []
    for arg in args:
        if arg.startswith(prefix):
            values.append(arg[len(prefix) :])
    return values


def validate_android_gradle_property_overrides(
    ctx: Context,
    app_name: str,
    gradle_args: list[str],
) -> tuple[bool, str]:
    if resolve_config_sync_target(ctx, app_name) != "android":
        return True, ""

    expected_source_root = normalize_cache_path(
        str(resolve_platform_config_source_root(ctx).resolve())
    )
    expected_output_root = normalize_cache_path(
        str(resolve_platform_config_output_root(ctx, "android").resolve())
    )

    source_overrides = extract_gradle_property_values(gradle_args, "timeTracerSourceConfigRoot")
    for value in source_overrides:
        actual = normalize_cache_path(value)
        if actual != expected_source_root:
            return (
                False,
                "Error: -PtimeTracerSourceConfigRoot must point to canonical "
                f"source config root ({resolve_platform_config_source_root(ctx)}). "
                f"Received: {value}",
            )

    output_overrides = extract_gradle_property_values(gradle_args, "timeTracerConfigRoot")
    for value in output_overrides:
        actual = normalize_cache_path(value)
        if actual != expected_output_root:
            return (
                False,
                "Error: -PtimeTracerConfigRoot must point to generated "
                f"android config root ({resolve_platform_config_output_root(ctx, 'android')}). "
                f"Received: {value}",
            )

    return True, ""


def resolve_android_config_gradle_property(ctx: Context, app_name: str) -> list[str]:
    if resolve_config_sync_target(ctx, app_name) != "android":
        return []
    output_root = resolve_platform_config_output_root(ctx, "android")
    return [f"-PtimeTracerConfigRoot={output_root}"]


def resolve_android_native_optimization_gradle_property(
    ctx: Context,
    app_name: str,
) -> list[str]:
    if resolve_config_sync_target(ctx, app_name) != "android":
        return []
    # Fast-compile-first policy: disable native Release optimization by default.
    return ["-PtimeTracerDisableNativeOptimization=true"]
