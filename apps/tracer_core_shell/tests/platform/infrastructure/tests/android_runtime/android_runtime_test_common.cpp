// infrastructure/tests/android_runtime/android_runtime_test_common.cpp
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

#include <sqlite3.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace android_runtime_tests {

auto Contains(const std::string& text, const std::string& keyword) -> bool {
  return text.find(keyword) != std::string::npos;
}

auto ExecuteSql(sqlite3* database, const std::string& sql) -> bool {
  char* error = nullptr;
  const int rc = sqlite3_exec(database, sql.c_str(), nullptr, nullptr, &error);
  if (rc != SQLITE_OK) {
    sqlite3_free(error);
    return false;
  }
  return true;
}

auto RemoveTree(const std::filesystem::path& path) -> void {
  std::error_code error;
  std::filesystem::remove_all(path, error);
}

auto BuildTempTestPaths(std::string_view test_name) -> RuntimeTestPaths {
  RuntimeTestPaths paths;
  paths.test_root =
      std::filesystem::temp_directory_path() / std::string(test_name);
  paths.output_root = paths.test_root / "output";
  paths.db_path = paths.output_root / "db" / "android.sqlite3";
  return paths;
}

auto BuildRuntimeRequest(
    const RuntimeTestPaths& paths,
    const std::filesystem::path& converter_config_toml_path)
    -> infrastructure::bootstrap::AndroidRuntimeRequest {
  std::filesystem::path resolved_converter_config =
      converter_config_toml_path;
  const std::filesystem::path repo_converter_config =
      BuildRepoRoot() / "config" / "user" / "behavior.toml";
  if (std::filesystem::absolute(converter_config_toml_path) ==
      std::filesystem::absolute(repo_converter_config)) {
    const std::filesystem::path test_config_root = paths.test_root / "config";
    if (!PrepareAndroidConfigFixture(test_config_root)) {
      throw std::runtime_error(
          "Failed to prepare Android runtime test config fixture");
    }
    resolved_converter_config = test_config_root / "user" / "behavior.toml";
  }

  infrastructure::bootstrap::AndroidRuntimeRequest request;
  request.output_root = paths.output_root;
  request.db_path = paths.db_path;
  request.converter_config_toml_path = resolved_converter_config;
  return request;
}

auto BuildBundleTomlPath(const std::filesystem::path& config_root)
    -> std::filesystem::path {
  return config_root / "program" / "meta" / "bundle.toml";
}

auto BuildRepoRoot() -> std::filesystem::path {
  return std::filesystem::path(__FILE__)
      .parent_path()   // android_runtime
      .parent_path()   // tests
      .parent_path()   // infrastructure
      .parent_path()   // platform
      .parent_path()   // tests
      .parent_path()   // tracer_core_shell
      .parent_path()   // apps
      .parent_path();  // repo root
}

namespace {

auto CopyFileWithParents(const std::filesystem::path& source_path,
                         const std::filesystem::path& target_path) -> bool {
  std::error_code error;
  std::filesystem::create_directories(target_path.parent_path(), error);
  if (error) {
    return false;
  }
  std::filesystem::copy_file(source_path, target_path,
                             std::filesystem::copy_options::overwrite_existing,
                             error);
  return !error;
}

auto CopyDirectoryTree(const std::filesystem::path& source_path,
                       const std::filesystem::path& target_path) -> bool {
  std::error_code error;
  std::filesystem::create_directories(target_path, error);
  if (error) {
    return false;
  }
  std::filesystem::copy(source_path, target_path,
                        std::filesystem::copy_options::recursive |
                            std::filesystem::copy_options::overwrite_existing,
                        error);
  return !error;
}

}  // namespace

auto WriteFileWithParents(const std::filesystem::path& target_path,
                          const std::string& content) -> bool {
  std::error_code error;
  std::filesystem::create_directories(target_path.parent_path(), error);
  if (error) {
    return false;
  }

  std::ofstream output(target_path, std::ios::trunc);
  if (!output.is_open()) {
    return false;
  }
  output << content;
  return static_cast<bool>(output);
}

auto PrepareAndroidConfigFixture(const std::filesystem::path& target_root)
    -> bool {
  const std::filesystem::path source_root =
      BuildRepoRoot() / "config" / "program";
  const std::filesystem::path user_source_root =
      BuildRepoRoot() / "config" / "user";
  const std::filesystem::path hierarchy_source_root =
      BuildRepoRoot() / "test" / "data" / "activity_hierarchy";
  const std::filesystem::path android_bundle_path =
      source_root / "meta" / "bundle.toml";
  const std::filesystem::path android_config_path =
      source_root / "config.toml";

  const auto copy_required_file = [&](std::string_view relative_path) -> bool {
    return CopyFileWithParents(
        source_root / std::filesystem::path(relative_path),
        target_root / "program" / std::filesystem::path(relative_path));
  };

  return CopyFileWithParents(android_bundle_path,
                             target_root / "program" / "meta" / "bundle.toml") &&
         CopyFileWithParents(android_config_path,
                             target_root / "program" / "config.toml") &&
         CopyFileWithParents(user_source_root / "behavior.toml",
                             target_root / "user" / "behavior.toml") &&
         CopyDirectoryTree(hierarchy_source_root,
                           target_root / "user" / "activity_hierarchy") &&
         copy_required_file("charts/heatmap.toml") &&
         copy_required_file("charts/pie.toml") &&
         CopyDirectoryTree(source_root / "insights" / "markdown",
                           target_root / "program" / "insights" / "markdown") &&
         CopyDirectoryTree(source_root / "insights" / "latex",
                           target_root / "program" / "insights" / "latex") &&
         CopyDirectoryTree(source_root / "insights" / "typst",
                           target_root / "program" / "insights" / "typst");
}

auto ExpectBuildRuntimeThrows(
    const infrastructure::bootstrap::AndroidRuntimeRequest& request,
    std::string& message) -> bool {
  try {
    static_cast<void>(infrastructure::bootstrap::BuildAndroidRuntime(request));
    return false;
  } catch (const std::exception& exception) {
    message = exception.what();
    return true;
  } catch (...) {
    message = "non-standard exception";
    return true;
  }
}

auto RunAndCheckInsightsQuery(
    const std::shared_ptr<ITracerCoreRuntime>& runtime_api,
    const tracer_core::core::dto::TemporalInsightsQueryRequest& request,
    std::string_view test_name, int& failures)
    -> std::optional<tracer_core::core::dto::TextOutput> {
  const auto result = runtime_api->insights().RunTemporalInsightsQuery(request);
  if (!result.ok) {
    ++failures;
    std::cerr << "[FAIL] RunTemporalInsightsQuery(" << test_name
              << ") should succeed: " << result.error_message << '\n';
    return std::nullopt;
  }
  return result;
}

}  // namespace android_runtime_tests
