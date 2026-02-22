// infrastructure/tests/android_runtime_test_common.hpp
#ifndef INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_TEST_COMMON_HPP_
#define INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_TEST_COMMON_HPP_

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "api/android/android_runtime_factory.hpp"
#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "application/use_cases/i_time_tracer_core_api.hpp"

struct sqlite3;

namespace android_runtime_tests {

struct RuntimeTestPaths {
  std::filesystem::path test_root;
  std::filesystem::path output_root;
  std::filesystem::path db_path;
};

auto Contains(const std::string& text, const std::string& keyword) -> bool;
auto ExecuteSql(sqlite3* database, const std::string& sql) -> bool;
auto RemoveTree(const std::filesystem::path& path) -> void;
auto BuildTempTestPaths(std::string_view test_name) -> RuntimeTestPaths;
auto BuildRuntimeRequest(
    const RuntimeTestPaths& paths,
    const std::filesystem::path& converter_config_toml_path)
    -> infrastructure::bootstrap::AndroidRuntimeRequest;
auto BuildBundleTomlPath(const std::filesystem::path& config_root)
    -> std::filesystem::path;
auto BuildRepoRoot() -> std::filesystem::path;
auto WriteFileWithParents(const std::filesystem::path& target_path,
                          const std::string& content) -> bool;
auto PrepareAndroidConfigFixture(const std::filesystem::path& target_root)
    -> bool;
auto ExpectBuildRuntimeThrows(
    const infrastructure::bootstrap::AndroidRuntimeRequest& request,
    std::string& message) -> bool;
auto RunAndCheckReportQuery(
    const std::shared_ptr<ITimeTracerCoreApi>& core_api,
    const time_tracer::core::dto::ReportQueryRequest& request,
    std::string_view test_name, int& failures)
    -> std::optional<time_tracer::core::dto::TextOutput>;

auto RunRuntimeSmokeTests(int& failures) -> void;
auto RunCoreConfigValidationTests(int& failures) -> void;
auto RunBusinessRegressionTests(int& failures) -> void;
auto RunAndroidBundlePolicyTests(int& failures) -> void;
auto RunCompatibilityTests(int& failures) -> void;
auto RunValidationIssueReporterTests(int& failures) -> void;
auto RunTxtMonthHeaderTests(int& failures) -> void;
auto RunDataQueryRefactorTests(int& failures) -> void;

}  // namespace android_runtime_tests

#endif  // INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_TEST_COMMON_HPP_
