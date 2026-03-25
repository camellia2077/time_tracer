#include "infrastructure/tests/android_runtime/android_runtime_smoke_internal.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>

namespace android_runtime_tests::smoke {

auto BuildRuntimeFixture(std::string_view test_name, int& failures)
    -> std::optional<RuntimeFixture> {
  RuntimeFixture fixture;
  fixture.paths = BuildTempTestPaths(test_name);

  const std::filesystem::path repo_root = BuildRepoRoot();
  fixture.input_path = repo_root / "test" / "data";
  fixture.config_toml_path = repo_root / "assets" / "tracer_core" / "config" /
                             "converter" / "interval_processor_config.toml";

  RemoveTree(fixture.paths.test_root);

  try {
    const auto request =
        BuildRuntimeRequest(fixture.paths, fixture.config_toml_path);
    fixture.runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should not throw: "
              << exception.what() << '\n';
    RemoveTree(fixture.paths.test_root);
    return std::nullopt;
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should not throw non-standard "
                 "exception.\n";
    RemoveTree(fixture.paths.test_root);
    return std::nullopt;
  }

  if (!fixture.runtime.runtime_api) {
    ++failures;
    std::cerr
        << "[FAIL] BuildAndroidRuntime should return a valid runtime API.\n";
    RemoveTree(fixture.paths.test_root);
    return std::nullopt;
  }

  return fixture;
}

auto CleanupRuntimeFixture(const RuntimeFixture& fixture) -> void {
  RemoveTree(fixture.paths.test_root);
}

}  // namespace android_runtime_tests::smoke
