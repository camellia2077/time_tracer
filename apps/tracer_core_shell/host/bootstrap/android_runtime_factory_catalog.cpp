// host/bootstrap/android_runtime_factory_catalog.cpp
#include "host/bootstrap/android_runtime_config_bridge.hpp"
#include "host/bootstrap/android_runtime_factory_internal.hpp"

namespace infrastructure::bootstrap::android_runtime_detail {

auto BuildAndroidInsightsCatalog(
    const std::filesystem::path& /*output_root*/,
    const AndroidRuntimeConfigPaths& runtime_config_paths) -> InsightsCatalog {
  return tracer_core::shell::config_bridge::BuildAndroidInsightsCatalogBridge(
      runtime_config_paths);
}

}  // namespace infrastructure::bootstrap::android_runtime_detail
