// infrastructure/tests/android_runtime/android_runtime_bundle_policy_tests.cpp
#include <filesystem>
#include <iostream>
#include <string>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

auto TestAndroidRuntimeRejectsBundleMissingRequiredFile(int& failures) -> void {
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "time_tracer_android_runtime_factory_bundle_missing_required_file_test");
  const std::filesystem::path kConfigRoot = paths.test_root / "config";
  const std::filesystem::path kConverterTomlPath =
      kConfigRoot / "activity_hierarchy" / "_system.toml";
  const std::filesystem::path kBundlePath = BuildBundleTomlPath(kConfigRoot);

  RemoveTree(paths.test_root);
  if (!PrepareAndroidConfigFixture(kConfigRoot)) {
    ++failures;
    std::cerr << "[FAIL] Failed to prepare Android config fixture for missing "
                 "required file test.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const std::string kBundleText = R"TOML(
schema_version = 1
profile = "android"

[file_list]
required = [
  "activity_hierarchy/missing-required.toml",
]
optional = []
)TOML";

  if (!WriteFileWithParents(kBundlePath, kBundleText)) {
    ++failures;
    std::cerr
        << "[FAIL] Failed to write bundle for missing required file test.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto request = BuildRuntimeRequest(paths, kConverterTomlPath);

  std::string message;
  const bool threw = ExpectBuildRuntimeThrows(request, message);
  if (!threw) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should reject bundle when "
                 "required file is missing.\n";
  } else if (!Contains(message, kBundlePath.string()) ||
             !Contains(message, "file_list.required[0]")) {
    ++failures;
    std::cerr << "[FAIL] Missing required file error should include bundle "
                 "path and field path, actual: "
              << message << '\n';
  }

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunAndroidBundlePolicyTests(int& failures) -> void {
  TestAndroidRuntimeRejectsBundleMissingRequiredFile(failures);
}

}  // namespace android_runtime_tests
