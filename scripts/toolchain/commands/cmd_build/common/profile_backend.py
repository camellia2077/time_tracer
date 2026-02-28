import os
from pathlib import Path

from ....core.config import BuildProfileConfig
from ....core.context import Context


def resolve_backend(ctx: Context, app_name: str) -> str:
    app = ctx.get_app_metadata(app_name)
    backend = (getattr(app, "backend", "cmake") or "cmake").strip().lower()
    if backend in {"cmake", "gradle"}:
        return backend
    print(f"Warning: unknown backend `{backend}` for app `{app_name}`; fallback to `cmake`.")
    return "cmake"


def resolve_build_dir_name(
    tidy: bool,
    build_dir_name: str | None,
    profile_build_dir: str | None = None,
) -> str:
    if build_dir_name:
        return build_dir_name
    if profile_build_dir:
        return profile_build_dir
    return "build_tidy" if tidy else "build_fast"


def resolve_profile(
    ctx: Context,
    profile_name: str | None,
) -> tuple[str | None, BuildProfileConfig | None]:
    explicit_profile = (profile_name or "").strip()
    default_profile = (ctx.config.build.default_profile or "").strip()
    selected_profile = explicit_profile or default_profile
    if not selected_profile:
        return None, None

    profile_cfg = ctx.config.build.profiles.get(selected_profile)
    if profile_cfg is None:
        print(
            "Warning: build profile "
            f"`{selected_profile}` not found in "
            "scripts/toolchain/config.toml. "
            "Profile settings will be ignored."
        )
        return None, None
    return selected_profile, profile_cfg


def profile_cmake_args(ctx: Context, profile_name: str | None) -> list[str]:
    _, profile_cfg = resolve_profile(ctx, profile_name)
    if profile_cfg is None:
        return []
    raw_args = getattr(profile_cfg, "cmake_args", []) or []
    return [arg for arg in raw_args if arg != "--"]


def profile_build_targets(ctx: Context, profile_name: str | None) -> list[str]:
    _, profile_cfg = resolve_profile(ctx, profile_name)
    if profile_cfg is None:
        return []
    raw_targets = getattr(profile_cfg, "build_targets", []) or []
    normalized_targets: list[str] = []
    for target in raw_targets:
        target_text = str(target).strip()
        if not target_text or target_text == "--":
            continue
        normalized_targets.append(target_text)
    return normalized_targets


def profile_gradle_tasks(ctx: Context, profile_name: str | None) -> list[str]:
    _, profile_cfg = resolve_profile(ctx, profile_name)
    if profile_cfg is None:
        return []
    raw_tasks = getattr(profile_cfg, "gradle_tasks", []) or []
    return [task for task in raw_tasks if task and task != "--"]


def profile_gradle_args(ctx: Context, profile_name: str | None) -> list[str]:
    _, profile_cfg = resolve_profile(ctx, profile_name)
    if profile_cfg is None:
        return []
    raw_args = getattr(profile_cfg, "gradle_args", []) or []
    return [arg for arg in raw_args if arg and arg != "--"]


def resolve_gradle_tasks(ctx: Context, app_name: str, profile_name: str | None) -> list[str]:
    profile_tasks = profile_gradle_tasks(ctx, profile_name)
    if profile_tasks:
        return profile_tasks

    app = ctx.get_app_metadata(app_name)
    app_tasks = [task for task in (getattr(app, "gradle_tasks", []) or []) if task]
    if app_tasks:
        return app_tasks

    return [":app:assembleDebug"]


def resolve_gradle_wrapper(ctx: Context, app_name: str) -> str:
    app = ctx.get_app_metadata(app_name)
    app_dir = ctx.get_app_dir(app_name)
    configured_wrapper = (getattr(app, "gradle_wrapper", "") or "").strip()
    if configured_wrapper:
        wrapper_path = Path(configured_wrapper)
        if not wrapper_path.is_absolute():
            wrapper_path = app_dir / wrapper_path
        if wrapper_path.exists():
            return str(wrapper_path)
        print(
            f"Warning: configured gradle wrapper does not exist: {wrapper_path}. "
            "Fallback to default wrapper name."
        )

    default_wrapper_name = "gradlew.bat" if os.name == "nt" else "gradlew"
    default_wrapper_path = app_dir / default_wrapper_name
    if default_wrapper_path.exists():
        return str(default_wrapper_path)

    print(f"Warning: gradle wrapper not found in {app_dir}. Fallback to `gradle` from PATH.")
    return "gradle"
