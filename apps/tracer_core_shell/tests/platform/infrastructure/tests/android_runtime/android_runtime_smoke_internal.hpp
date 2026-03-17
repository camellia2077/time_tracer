// infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp
#ifndef INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_INTERNAL_HPP_
#define INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_INTERNAL_HPP_

#include <filesystem>
#include <optional>
#include <string_view>

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

auto RunBootstrapSmokeSection(int& failures) -> void;
auto RunConfigSmokeSection(int& failures) -> void;
auto RunIoSmokeSection(int& failures) -> void;

}  // namespace android_runtime_tests::smoke

#endif  // INFRASTRUCTURE_TESTS_ANDROID_RUNTIME_ANDROID_RUNTIME_SMOKE_INTERNAL_HPP_
