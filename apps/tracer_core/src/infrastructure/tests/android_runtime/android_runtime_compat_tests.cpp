// infrastructure/tests/android_runtime/android_runtime_compat_tests.cpp
#include <exception>
#include <filesystem>
#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

auto TestAndroidRuntimeFallsBackToLegacyConfigPathsWhenBundleMissing(
    int& failures) -> void {
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "time_tracer_android_runtime_factory_legacy_fallback_test");
  const std::filesystem::path kLegacyConfigRoot = paths.test_root / "config";
  const std::filesystem::path kConverterTomlPath =
      kLegacyConfigRoot / "converter" / "interval_processor_config.toml";
  RemoveTree(paths.test_root);
  if (!PrepareAndroidConfigFixture(kLegacyConfigRoot)) {
    ++failures;
    std::cerr << "[FAIL] Failed to prepare legacy Android config fixtures.\n";
    RemoveTree(paths.test_root);
    return;
  }

  try {
    const auto request = BuildRuntimeRequest(paths, kConverterTomlPath);

    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.core_api) {
      ++failures;
      std::cerr << "[FAIL] Legacy fallback should build Android runtime core "
                   "API.\n";
      RemoveTree(paths.test_root);
      return;
    }

    static_cast<void>(RunAndCheckReportQuery(
        runtime.core_api,
        {.type = tracer_core::core::dto::ReportQueryType::kDay,
         .argument = "2026-02-01",
         .format = ReportFormat::kMarkdown},
        "legacy day markdown", failures));
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Legacy fallback test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Legacy fallback test threw non-standard exception.\n";
  }

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunCompatibilityTests(int& failures) -> void {
  TestAndroidRuntimeFallsBackToLegacyConfigPathsWhenBundleMissing(failures);
}

}  // namespace android_runtime_tests
