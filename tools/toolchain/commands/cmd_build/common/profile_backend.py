import os
from pathlib import Path
from typing import Iterable

from ....core.config import BuildProfileConfig
from ....core.context import Context


def resolve_backend(ctx: Context, app_name: str) -> str:
    app = ctx.get_app_metadata(app_name)
    backend = (getattr(app, "backend", "cmake") or "cmake").strip().lower()
    if backend in {"cmake", "gradle", "cargo"}:
        return backend
    print(f"Warning: unknown backend `{backend}` for app `{app_name}`; fallback to `cmake`.")
    return "cmake"


def resolve_fixed_build_dir(ctx: Context, app_name: str) -> str | None:
    app = ctx.get_app_metadata(app_name)
    fixed_build_dir = (getattr(app, "fixed_build_dir", "") or "").strip()
    if fixed_build_dir:
        return fixed_build_dir
    return None


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
            "tools/toolchain/config/*.toml. "
            "Profile settings will be ignored."
        )
        return None, None
    return selected_profile, profile_cfg


def _normalize_profile_names(profile_name: str | Iterable[str] | None) -> list[str]:
    if profile_name is None:
        return []
    if isinstance(profile_name, str):
        selected = profile_name.strip()
        return [selected] if selected else []

    normalized: list[str] = []
    seen: set[str] = set()
    for raw_profile in profile_name:
        selected = str(raw_profile or "").strip()
        if not selected or selected in seen:
            continue
        seen.add(selected)
        normalized.append(selected)
    return normalized


def resolve_profiles(
    ctx: Context,
    profile_name: str | Iterable[str] | None,
) -> list[tuple[str, BuildProfileConfig]]:
    resolved: list[tuple[str, BuildProfileConfig]] = []
    for selected_profile in _normalize_profile_names(profile_name):
        profile_cfg = ctx.config.build.profiles.get(selected_profile)
        if profile_cfg is None:
            print(
                "Warning: build profile "
                f"`{selected_profile}` not found in "
                "tools/toolchain/config/*.toml. "
                "Profile settings will be ignored."
            )
            continue
        resolved.append((selected_profile, profile_cfg))
    return resolved


def _merge_profile_list_values(
    profiles: list[tuple[str, BuildProfileConfig]],
    *,
    field_name: str,
    dedupe_values: bool = True,
) -> list[str]:
    merged: list[str] = []
    seen: set[str] = set()
    for _, profile_cfg in profiles:
        raw_values = getattr(profile_cfg, field_name, []) or []
        for raw_value in raw_values:
            value = str(raw_value).strip()
            if not value or value == "--":
                continue
            if dedupe_values and value in seen:
                continue
            if dedupe_values:
                seen.add(value)
            merged.append(value)
    return merged


def profile_cmake_args(ctx: Context, profile_name: str | Iterable[str] | None) -> list[str]:
    profiles = resolve_profiles(ctx, profile_name)
    if profiles:
        return _merge_profile_list_values(
            profiles,
            field_name="cmake_args",
            dedupe_values=False,
        )
    _, profile_cfg = resolve_profile(ctx, profile_name if isinstance(profile_name, str) else None)
    if profile_cfg is None:
        return []
    raw_args = getattr(profile_cfg, "cmake_args", []) or []
    return [arg for arg in raw_args if arg != "--"]


def profile_build_targets(ctx: Context, profile_name: str | Iterable[str] | None) -> list[str]:
    profiles = resolve_profiles(ctx, profile_name)
    if profiles:
        return _merge_profile_list_values(profiles, field_name="build_targets")
    _, profile_cfg = resolve_profile(ctx, profile_name if isinstance(profile_name, str) else None)
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


def profile_gradle_tasks(ctx: Context, profile_name: str | Iterable[str] | None) -> list[str]:
    profiles = resolve_profiles(ctx, profile_name)
    if profiles:
        return _merge_profile_list_values(profiles, field_name="gradle_tasks")
    _, profile_cfg = resolve_profile(ctx, profile_name if isinstance(profile_name, str) else None)
    if profile_cfg is None:
        return []
    raw_tasks = getattr(profile_cfg, "gradle_tasks", []) or []
    return [task for task in raw_tasks if task and task != "--"]


def profile_gradle_args(ctx: Context, profile_name: str | Iterable[str] | None) -> list[str]:
    profiles = resolve_profiles(ctx, profile_name)
    if profiles:
        return _merge_profile_list_values(
            profiles,
            field_name="gradle_args",
            dedupe_values=False,
        )
    _, profile_cfg = resolve_profile(ctx, profile_name if isinstance(profile_name, str) else None)
    if profile_cfg is None:
        return []
    raw_args = getattr(profile_cfg, "gradle_args", []) or []
    return [arg for arg in raw_args if arg and arg != "--"]


def profile_cargo_args(ctx: Context, profile_name: str | Iterable[str] | None) -> list[str]:
    profiles = resolve_profiles(ctx, profile_name)
    if profiles:
        return _merge_profile_list_values(
            profiles,
            field_name="cargo_args",
            dedupe_values=False,
        )
    _, profile_cfg = resolve_profile(ctx, profile_name if isinstance(profile_name, str) else None)
    if profile_cfg is None:
        return []
    raw_args = getattr(profile_cfg, "cargo_args", []) or []
    return [arg for arg in raw_args if arg and arg != "--"]


def resolve_gradle_tasks(
    ctx: Context,
    app_name: str,
    profile_name: str | Iterable[str] | None,
) -> list[str]:
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
