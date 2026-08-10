#ifndef HOST_ANDROID_RUNTIME_CONFIG_BRIDGE_H_
#define HOST_ANDROID_RUNTIME_CONFIG_BRIDGE_H_

#include "host/bootstrap/android_runtime_factory_internal.hpp"

namespace tracer_core::shell::config_bridge {

[[nodiscard]] auto ResolveAndroidRuntimeConfigPathsBridge(
    const std::filesystem::path& requested_converter_config_toml_path)
    -> ::infrastructure::bootstrap::android_runtime_detail::
        AndroidRuntimeConfigPaths;

[[nodiscard]] auto BuildAndroidInsightsCatalogBridge(
    const ::infrastructure::bootstrap::android_runtime_detail::
        AndroidRuntimeConfigPaths& runtime_config_paths) -> InsightsCatalog;

}  // namespace tracer_core::shell::config_bridge

#endif  // HOST_ANDROID_RUNTIME_CONFIG_BRIDGE_H_
