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
  fixture.config_toml_path = fixture.paths.test_root / "config" / "user" /
                             "behavior.toml";

  RemoveTree(fixture.paths.test_root);

  try {
    std::error_code io_error;
    const auto config_root = fixture.paths.test_root / "config";
    std::filesystem::create_directories(config_root / "user", io_error);
    std::filesystem::copy(
        repo_root / "config" / "program", config_root / "program",
        std::filesystem::copy_options::recursive |
            std::filesystem::copy_options::overwrite_existing,
        io_error);
    std::filesystem::copy_file(
        repo_root / "config" / "user" / "behavior.toml",
        fixture.config_toml_path,
        std::filesystem::copy_options::overwrite_existing, io_error);
    std::filesystem::copy(
        repo_root / "test" / "data" / "activity_hierarchy",
        config_root / "user" / "activity_hierarchy",
        std::filesystem::copy_options::recursive |
            std::filesystem::copy_options::overwrite_existing,
        io_error);
    if (io_error) {
      throw std::runtime_error("Failed to prepare Android runtime config: " +
                               io_error.message());
    }
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
