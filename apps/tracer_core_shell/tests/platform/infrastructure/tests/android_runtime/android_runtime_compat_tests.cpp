// infrastructure/tests/android_runtime/android_runtime_compat_tests.cpp
#include <exception>
#include <filesystem>
#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

auto CopyFixtureFile(const std::filesystem::path& relative_path,
                     const std::filesystem::path& target_path) -> bool {
  std::error_code error;
  std::filesystem::create_directories(target_path.parent_path(), error);
  if (error) {
    return false;
  }

  std::filesystem::copy_file(BuildRepoRoot() / relative_path, target_path,
                             std::filesystem::copy_options::overwrite_existing,
                             error);
  return !error;
}

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
    if (!runtime.runtime_api) {
      ++failures;
      std::cerr << "[FAIL] Legacy fallback should build Android runtime core "
                   "runtime API.\n";
      RemoveTree(paths.test_root);
      return;
    }

    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = (BuildRepoRoot() / "test" / "data").string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    const auto ingest_result =
        runtime.runtime_api->pipeline().RunIngest(ingest_request);
    if (!ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] Legacy fallback test ingest should succeed: "
                << ingest_result.error_message << '\n';
      RemoveTree(paths.test_root);
      return;
    }

    static_cast<void>(RunAndCheckReportQuery(
        runtime.runtime_api,
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

auto TestAndroidRuntimeRejectsLegacyAliasMappingFixture(int& failures) -> void {
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "time_tracer_android_runtime_factory_legacy_alias_fixture_test");
  const std::filesystem::path kLegacyConfigRoot = paths.test_root / "config";
  const std::filesystem::path kConverterTomlPath =
      kLegacyConfigRoot / "converter" / "interval_processor_config.toml";
  const std::filesystem::path kAliasMappingPath =
      kLegacyConfigRoot / "converter" / "alias_mapping.toml";
  RemoveTree(paths.test_root);
  if (!PrepareAndroidConfigFixture(kLegacyConfigRoot)) {
    ++failures;
    std::cerr << "[FAIL] Failed to prepare config fixtures for legacy alias "
                 "compat test.\n";
    RemoveTree(paths.test_root);
    return;
  }
  if (!CopyFixtureFile("test/fixtures/config/legacy/alias_mapping.legacy.toml",
                       kAliasMappingPath)) {
    ++failures;
    std::cerr << "[FAIL] Legacy alias compat test should overwrite alias "
                 "mapping with the legacy fixture.\n";
    RemoveTree(paths.test_root);
    return;
  }

  std::string message;
  const bool threw =
      ExpectBuildRuntimeThrows(BuildRuntimeRequest(paths, kConverterTomlPath),
                               message);
  if (!threw) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should reject legacy single-file "
                 "alias mapping fixtures that no longer match the index-based "
                 "config format.\n";
    RemoveTree(paths.test_root);
    return;
  }

  if (!Contains(message, "Converter config validation failed")) {
    ++failures;
    std::cerr << "[FAIL] Legacy alias compat failure should explain that the "
                 "legacy alias fixture is rejected during converter config "
                 "validation, actual: "
              << message << '\n';
  }

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunCompatibilityTests(int& failures) -> void {
  TestAndroidRuntimeFallsBackToLegacyConfigPathsWhenBundleMissing(failures);
  TestAndroidRuntimeRejectsLegacyAliasMappingFixture(failures);
}

}  // namespace android_runtime_tests
