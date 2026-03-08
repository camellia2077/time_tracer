from __future__ import annotations


def resolve_suite_config_override(
    suite_name: str,
    profile_name: str | None,
) -> str | None:
    normalized_profile = (profile_name or "").strip().lower()
    if not normalized_profile:
        return None

    if suite_name == "tracer_android":
        if normalized_profile == "android_style":
            return "config_android_style.toml"
        if normalized_profile == "android_ci":
            return "config_android_ci.toml"
        if normalized_profile == "android_device":
            return "config_android_device.toml"
    return None
