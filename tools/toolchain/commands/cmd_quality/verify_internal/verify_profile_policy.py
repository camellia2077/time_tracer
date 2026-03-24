from __future__ import annotations

_TRACER_WINDOWS_RUST_CLI_PROFILE_CONFIGS = {
    "cap_pipeline": "config_cap_pipeline.toml",
    "cap_query": "config_cap_query.toml",
    "cap_reporting": "config_cap_reporting.toml",
    "cap_exchange": "config_cap_exchange.toml",
    "cap_config": "config_cap_config.toml",
    "cap_persistence_runtime": "config_cap_persistence_runtime.toml",
    "cap_persistence_write": "config_cap_persistence_write.toml",
}


def should_run_reporting_markdown_gates(profile_name: str | None) -> bool:
    normalized_profile = (profile_name or "").strip().lower()
    if not normalized_profile:
        return True

    if normalized_profile == "cap_reporting":
        return True

    return not normalized_profile.startswith("cap_")


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
    if suite_name == "tracer_windows_rust_cli":
        return _TRACER_WINDOWS_RUST_CLI_PROFILE_CONFIGS.get(normalized_profile)
    return None
