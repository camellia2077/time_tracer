from __future__ import annotations

from collections.abc import Sequence

_TRACER_WINDOWS_RUST_CLI_PROFILE_CONFIGS = {
    "cap_pipeline": "config_cap_pipeline.toml",
    "cap_query": "config_cap_query.toml",
    "cap_insights": "config_cap_insights.toml",
    "cap_exchange": "config_cap_exchange.toml",
    "cap_config": "config_cap_config.toml",
    "cap_persistence_runtime": "config_cap_persistence_runtime.toml",
    "cap_persistence_write": "config_cap_persistence_write.toml",
    "shell_aggregate": "config_shell_aggregate.toml",
}


def _normalize_profile_names(profile_name: str | Sequence[str] | None) -> tuple[str, ...]:
    if profile_name is None:
        return ()
    raw_profiles = (profile_name,) if isinstance(profile_name, str) else profile_name
    return tuple(profile.strip().lower() for profile in raw_profiles if profile.strip())


def should_run_insights_markdown_gates(
    profile_name: str | Sequence[str] | None,
) -> bool:
    normalized_profiles = _normalize_profile_names(profile_name)
    if not normalized_profiles:
        return True

    if "cap_insights" in normalized_profiles:
        return True

    if all(profile == "shell_aggregate" for profile in normalized_profiles):
        return False

    return any(not profile.startswith("cap_") for profile in normalized_profiles)


def resolve_suite_config_override(
    suite_name: str,
    profile_name: str | Sequence[str] | None,
) -> str | None:
    normalized_profiles = _normalize_profile_names(profile_name)
    if not normalized_profiles:
        return None

    if suite_name == "tracer_android":
        android_configs = {
            "android_style": "config_android_style.toml",
            "android_ci": "config_android_ci.toml",
            "android_release_verify": "config_android_release_verify.toml",
            "android_release_device": "config_android_release_device.toml",
            "android_device": "config_android_device.toml",
        }
        for profile in reversed(normalized_profiles):
            if profile in android_configs:
                return android_configs[profile]
    if suite_name == "tracer_windows_rust_cli":
        for profile in reversed(normalized_profiles):
            if profile in _TRACER_WINDOWS_RUST_CLI_PROFILE_CONFIGS:
                return _TRACER_WINDOWS_RUST_CLI_PROFILE_CONFIGS[profile]
    return None
