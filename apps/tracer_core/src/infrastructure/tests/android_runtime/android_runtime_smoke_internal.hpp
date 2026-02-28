// infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp
#ifndef INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_INTERNAL_HPP_
#define INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_INTERNAL_HPP_

#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests::smoke {

struct RuntimeFixture {
  RuntimeTestPaths paths;
  std::filesystem::path input_path;
  std::filesystem::path config_toml_path;
  infrastructure::bootstrap::AndroidRuntime runtime;
};

[[nodiscard]] auto BuildRuntimeFixture(std::string_view test_name,
                                       int& failures)
    -> std::optional<RuntimeFixture>;

auto CleanupRuntimeFixture(const RuntimeFixture& fixture) -> void;

[[nodiscard]] auto ParseJsonOrRecordFailure(const std::string& content,
                                            std::string_view context,
                                            int& failures)
    -> std::optional<nlohmann::json>;

auto ValidateChartSeriesPayload(const nlohmann::json& payload,
                                std::string_view context, int& failures)
    -> void;

[[nodiscard]] auto RunDataQueryOrRecordFailure(
    const std::shared_ptr<ITracerCoreApi>& core_api,
    const tracer_core::core::dto::DataQueryRequest& request,
    std::string_view context, int& failures)
    -> std::optional<tracer_core::core::dto::TextOutput>;

auto RunBootstrapSmokeSection(int& failures) -> void;
auto RunConfigSmokeSection(int& failures) -> void;
auto RunIoSmokeSection(int& failures) -> void;

}  // namespace android_runtime_tests::smoke

#endif  // INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_INTERNAL_HPP_
