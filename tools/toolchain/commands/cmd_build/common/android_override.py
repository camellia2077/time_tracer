from ....core.context import Context


def resolve_android_gradle_platform(ctx: Context, app_name: str) -> str | None:
    app = ctx.get_app_metadata(app_name)
    raw_platform = (getattr(app, "gradle_platform", "") or "").strip().lower()
    if not raw_platform:
        return None
    if raw_platform == "android":
        return raw_platform
    print(
        f"Warning: unknown gradle_platform `{raw_platform}` for app "
        f"`{app_name}`; skipping Android-specific Gradle overrides."
    )
    return None


def resolve_android_native_optimization_gradle_args(extra_args: list[str]) -> bool:
    return any(arg.startswith("-PtimeTracerDisableNativeOptimization=") for arg in extra_args)


def resolve_android_native_optimization_gradle_property(
    ctx: Context,
    app_name: str,
) -> list[str]:
    if resolve_android_gradle_platform(ctx, app_name) != "android":
        return []
    # Fast-compile-first policy: disable native Release optimization by default.
    return ["-PtimeTracerDisableNativeOptimization=true"]
