// infrastructure/tests/android_runtime/android_runtime_bundle_policy_tests.cpp
#include <filesystem>
#include <iostream>
#include <string>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

auto TestAndroidRuntimeRejectsAndroidBundleWithLatexReports(int& failures)
    -> void {
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "time_tracer_android_runtime_factory_android_latex_forbidden_test");
  const std::filesystem::path kConfigRoot = paths.test_root / "config";
  const std::filesystem::path kConverterTomlPath =
      kConfigRoot / "converter" / "interval_processor_config.toml";
  const std::filesystem::path kBundlePath = BuildBundleTomlPath(kConfigRoot);

  RemoveTree(paths.test_root);
  if (!PrepareAndroidConfigFixture(kConfigRoot)) {
    ++failures;
    std::cerr << "[FAIL] Failed to prepare Android config fixture for "
                 "latex forbidden test.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const std::string kBundleText = R"TOML(
schema_version = 1
profile = "android"

[file_list]
required = [
  "converter/interval_processor_config.toml",
  "charts/heatmap.toml",
  "reports/markdown/day.toml",
  "reports/markdown/month.toml",
  "reports/markdown/period.toml",
  "reports/markdown/week.toml",
  "reports/markdown/year.toml",
]
optional = []

[paths.converter]
interval_config = "converter/interval_processor_config.toml"

[paths.visualization]
heatmap = "charts/heatmap.toml"

[paths.reports.markdown]
day = "reports/markdown/day.toml"
month = "reports/markdown/month.toml"
period = "reports/markdown/period.toml"
week = "reports/markdown/week.toml"
year = "reports/markdown/year.toml"

[paths.reports.latex]
day = "reports/markdown/day.toml"
month = "reports/markdown/month.toml"
period = "reports/markdown/period.toml"
week = "reports/markdown/week.toml"
year = "reports/markdown/year.toml"
)TOML";

  if (!WriteFileWithParents(kBundlePath, kBundleText)) {
    ++failures;
    std::cerr << "[FAIL] Failed to write bundle for latex forbidden test.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto request = BuildRuntimeRequest(paths, kConverterTomlPath);

  std::string message;
  const bool threw = ExpectBuildRuntimeThrows(request, message);
  if (!threw) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should reject android bundle "
                 "with paths.reports.latex.\n";
  } else if (!Contains(message, kBundlePath.string()) ||
             !Contains(message, "paths.reports.latex")) {
    ++failures;
    std::cerr << "[FAIL] Latex forbidden error should include bundle path "
                 "and field path, actual: "
              << message << '\n';
  }

  RemoveTree(paths.test_root);
}

auto TestAndroidRuntimeRejectsAndroidBundleWithTypstReports(int& failures)
    -> void {
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "time_tracer_android_runtime_factory_android_typst_forbidden_test");
  const std::filesystem::path kConfigRoot = paths.test_root / "config";
  const std::filesystem::path kConverterTomlPath =
      kConfigRoot / "converter" / "interval_processor_config.toml";
  const std::filesystem::path kBundlePath = BuildBundleTomlPath(kConfigRoot);

  RemoveTree(paths.test_root);
  if (!PrepareAndroidConfigFixture(kConfigRoot)) {
    ++failures;
    std::cerr << "[FAIL] Failed to prepare Android config fixture for "
                 "typst forbidden test.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const std::string kBundleText = R"TOML(
schema_version = 1
profile = "android"

[file_list]
required = [
  "converter/interval_processor_config.toml",
  "charts/heatmap.toml",
  "reports/markdown/day.toml",
  "reports/markdown/month.toml",
  "reports/markdown/period.toml",
  "reports/markdown/week.toml",
  "reports/markdown/year.toml",
]
optional = []

[paths.converter]
interval_config = "converter/interval_processor_config.toml"

[paths.visualization]
heatmap = "charts/heatmap.toml"

[paths.reports.markdown]
day = "reports/markdown/day.toml"
month = "reports/markdown/month.toml"
period = "reports/markdown/period.toml"
week = "reports/markdown/week.toml"
year = "reports/markdown/year.toml"

[paths.reports.typst]
day = "reports/markdown/day.toml"
month = "reports/markdown/month.toml"
period = "reports/markdown/period.toml"
week = "reports/markdown/week.toml"
year = "reports/markdown/year.toml"
)TOML";

  if (!WriteFileWithParents(kBundlePath, kBundleText)) {
    ++failures;
    std::cerr << "[FAIL] Failed to write bundle for typst forbidden test.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto request = BuildRuntimeRequest(paths, kConverterTomlPath);

  std::string message;
  const bool threw = ExpectBuildRuntimeThrows(request, message);
  if (!threw) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should reject android bundle "
                 "with paths.reports.typst.\n";
  } else if (!Contains(message, kBundlePath.string()) ||
             !Contains(message, "paths.reports.typst")) {
    ++failures;
    std::cerr << "[FAIL] Typst forbidden error should include bundle path "
                 "and field path, actual: "
              << message << '\n';
  }

  RemoveTree(paths.test_root);
}

auto TestAndroidRuntimeRejectsAndroidBundleWithoutMarkdown(int& failures)
    -> void {
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "time_tracer_android_runtime_factory_android_missing_markdown_test");
  const std::filesystem::path kConfigRoot = paths.test_root / "config";
  const std::filesystem::path kConverterTomlPath =
      kConfigRoot / "converter" / "interval_processor_config.toml";
  const std::filesystem::path kBundlePath = BuildBundleTomlPath(kConfigRoot);

  RemoveTree(paths.test_root);
  if (!PrepareAndroidConfigFixture(kConfigRoot)) {
    ++failures;
    std::cerr << "[FAIL] Failed to prepare Android config fixture for "
                 "missing markdown test.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const std::string kBundleText = R"TOML(
schema_version = 1
profile = "android"

[file_list]
required = [
  "converter/interval_processor_config.toml",
  "charts/heatmap.toml",
  "reports/markdown/day.toml",
  "reports/markdown/month.toml",
  "reports/markdown/period.toml",
  "reports/markdown/week.toml",
  "reports/markdown/year.toml",
]
optional = []

[paths.converter]
interval_config = "converter/interval_processor_config.toml"

[paths.visualization]
heatmap = "charts/heatmap.toml"

[paths.reports]
)TOML";

  if (!WriteFileWithParents(kBundlePath, kBundleText)) {
    ++failures;
    std::cerr << "[FAIL] Failed to write bundle for missing markdown test.\n";
    RemoveTree(paths.test_root);
    return;
  }

  const auto request = BuildRuntimeRequest(paths, kConverterTomlPath);

  std::string message;
  const bool threw = ExpectBuildRuntimeThrows(request, message);
  if (!threw) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should reject android bundle "
                 "without paths.reports.markdown.\n";
  } else if (!Contains(message, kBundlePath.string()) ||
             !Contains(message, "paths.reports.markdown")) {
    ++failures;
    std::cerr << "[FAIL] Missing markdown error should include bundle path "
                 "and field path, actual: "
              << message << '\n';
  }

  RemoveTree(paths.test_root);
}

auto TestAndroidRuntimeRejectsBundleMissingRequiredFile(int& failures) -> void {
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "time_tracer_android_runtime_factory_bundle_missing_required_file_test");
  const std::filesystem::path kConfigRoot = paths.test_root / "config";
  const std::filesystem::path kConverterTomlPath =
      kConfigRoot / "converter" / "interval_processor_config.toml";
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
  "converter/missing-required.toml",
]
optional = []

[paths.converter]
interval_config = "converter/interval_processor_config.toml"

[paths.visualization]
heatmap = "charts/heatmap.toml"

[paths.reports.markdown]
day = "reports/markdown/day.toml"
month = "reports/markdown/month.toml"
period = "reports/markdown/period.toml"
week = "reports/markdown/week.toml"
year = "reports/markdown/year.toml"
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
  TestAndroidRuntimeRejectsAndroidBundleWithLatexReports(failures);
  TestAndroidRuntimeRejectsAndroidBundleWithTypstReports(failures);
  TestAndroidRuntimeRejectsAndroidBundleWithoutMarkdown(failures);
  TestAndroidRuntimeRejectsBundleMissingRequiredFile(failures);
}

}  // namespace android_runtime_tests
