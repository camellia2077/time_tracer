// host/bootstrap/android_runtime_factory_catalog.cpp
#include "host/bootstrap/android_runtime_config_bridge.hpp"
#include "host/bootstrap/android_runtime_factory_internal.hpp"

namespace infrastructure::bootstrap::android_runtime_detail {

auto BuildAndroidReportCatalog(
    const std::filesystem::path& /*output_root*/,
    const AndroidRuntimeConfigPaths& runtime_config_paths) -> ReportCatalog {
  return tracer_core::shell::config_bridge::BuildAndroidReportCatalogBridge(
      runtime_config_paths);
}

}  // namespace infrastructure::bootstrap::android_runtime_detail
