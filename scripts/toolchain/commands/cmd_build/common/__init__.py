from .android_override import (
    extract_gradle_property_values,
    resolve_android_config_gradle_args,
    resolve_android_config_gradle_property,
    resolve_android_native_optimization_gradle_args,
    resolve_android_native_optimization_gradle_property,
    validate_android_gradle_property_overrides,
)
from .config_sync import (
    resolve_config_sync_target,
    resolve_platform_config_output_root,
    resolve_platform_config_runner_path,
    resolve_platform_config_source_root,
    sync_platform_config_if_needed,
)
from .flags import has_cmake_definition, resolve_toolchain_flags
from .profile_backend import (
    profile_cmake_args,
    profile_gradle_args,
    profile_gradle_tasks,
    resolve_backend,
    resolve_build_dir_name,
    resolve_gradle_tasks,
    resolve_gradle_wrapper,
    resolve_profile,
)
from .utils import normalize_cache_path
from .windows_override import (
    extract_cmake_definition_values,
    resolve_windows_config_cmake_args,
    strip_cmake_definition,
    sync_windows_runtime_config_copy_if_needed,
    validate_windows_config_source_override,
)

__all__ = [
    "resolve_platform_config_source_root",
    "resolve_platform_config_output_root",
    "resolve_platform_config_runner_path",
    "resolve_config_sync_target",
    "sync_platform_config_if_needed",
    "resolve_windows_config_cmake_args",
    "extract_cmake_definition_values",
    "strip_cmake_definition",
    "validate_windows_config_source_override",
    "sync_windows_runtime_config_copy_if_needed",
    "resolve_android_config_gradle_args",
    "extract_gradle_property_values",
    "validate_android_gradle_property_overrides",
    "resolve_android_config_gradle_property",
    "resolve_android_native_optimization_gradle_args",
    "resolve_android_native_optimization_gradle_property",
    "resolve_backend",
    "resolve_build_dir_name",
    "resolve_profile",
    "profile_cmake_args",
    "profile_gradle_args",
    "profile_gradle_tasks",
    "resolve_gradle_tasks",
    "resolve_gradle_wrapper",
    "normalize_cache_path",
    "has_cmake_definition",
    "resolve_toolchain_flags",
]
