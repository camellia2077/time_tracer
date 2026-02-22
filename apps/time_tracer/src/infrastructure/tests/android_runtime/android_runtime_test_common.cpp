// infrastructure/tests/android_runtime_test_common.cpp
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
  infrastructure::bootstrap::AndroidRuntimeRequest request;
  request.output_root = paths.output_root;
  request.db_path = paths.db_path;
  request.converter_config_toml_path = converter_config_toml_path;
  return request;
}

auto BuildBundleTomlPath(const std::filesystem::path& config_root)
    -> std::filesystem::path {
  return config_root / "meta" / "bundle.toml";
}

auto BuildRepoRoot() -> std::filesystem::path {
  return std::filesystem::path(__FILE__)
      .parent_path()   // tests
      .parent_path()   // infrastructure
      .parent_path()   // src
      .parent_path()   // time_tracer
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
      BuildRepoRoot() / "apps" / "time_tracer" / "config";

  const auto copy_required_file = [&](std::string_view relative_path) -> bool {
    return CopyFileWithParents(
        source_root / std::filesystem::path(relative_path),
        target_root / std::filesystem::path(relative_path));
  };

  return copy_required_file("converter/interval_processor_config.toml") &&
         copy_required_file("converter/alias_mapping.toml") &&
         copy_required_file("converter/duration_rules.toml") &&
         copy_required_file("reports/markdown/day.toml") &&
         copy_required_file("reports/markdown/month.toml") &&
         copy_required_file("reports/markdown/period.toml") &&
         copy_required_file("reports/markdown/week.toml") &&
         copy_required_file("reports/markdown/year.toml");
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

auto RunAndCheckReportQuery(
    const std::shared_ptr<ITimeTracerCoreApi>& core_api,
    const time_tracer::core::dto::ReportQueryRequest& request,
    std::string_view test_name, int& failures)
    -> std::optional<time_tracer::core::dto::TextOutput> {
  const auto result = core_api->RunReportQuery(request);
  if (!result.ok) {
    ++failures;
    std::cerr << "[FAIL] RunReportQuery(" << test_name
              << ") should succeed: " << result.error_message << '\n';
    return std::nullopt;
  }
  return result;
}

}  // namespace android_runtime_tests
