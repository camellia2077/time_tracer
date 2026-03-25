// host/bootstrap/android_runtime_factory_resolver.cpp
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "host/bootstrap/android_runtime_config_bridge.hpp"
#include "host/bootstrap/android_runtime_factory_internal.hpp"

namespace infrastructure::bootstrap::android_runtime_detail {

namespace {

namespace fs = std::filesystem;

constexpr std::string_view kDatabaseFilename = "time_data.sqlite3";

}  // namespace


auto ResolveOutputRoot(const fs::path& output_root) -> fs::path {
  if (output_root.empty()) {
    throw std::invalid_argument(
        "Android runtime output_root must not be empty.");
  }
  return fs::absolute(output_root);
}

auto ResolveDbPath(const fs::path& db_path, const fs::path& output_root)
    -> fs::path {
  if (!db_path.empty()) {
    return fs::absolute(db_path);
  }
  return fs::absolute(output_root / "db" / kDatabaseFilename);
}

auto ResolveAndroidRuntimeConfigPaths(
    const fs::path& requested_converter_config_toml_path)
    -> AndroidRuntimeConfigPaths {
  return tracer_core::shell::config_bridge::ResolveAndroidRuntimeConfigPathsBridge(
      requested_converter_config_toml_path);
}

}  // namespace infrastructure::bootstrap::android_runtime_detail
