// infrastructure/tests/android_runtime_core_config_tests.cpp
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "infrastructure/config/loader/report_config_loader.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

auto TestAndroidRuntimeRejectsInvalidConverterConfig(int& failures) -> void {
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "time_tracer_android_runtime_factory_invalid_config_test");
  const std::filesystem::path kInvalidConfigPath =
      paths.test_root / "invalid.toml";

  RemoveTree(paths.test_root);
  std::filesystem::create_directories(paths.test_root);

  {
    std::ofstream file(kInvalidConfigPath);
    file << "remark_prefix = \"r \"\n";
  }

  bool threw = false;
  try {
    const auto request = BuildRuntimeRequest(paths, kInvalidConfigPath);
    static_cast<void>(infrastructure::bootstrap::BuildAndroidRuntime(request));
  } catch (const std::exception&) {
    threw = true;
  }

  if (!threw) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should fail for invalid converter "
                 "config TOML.\n";
  }

  RemoveTree(paths.test_root);
}

auto TestReportConfigLoaderRejectsInvalidDailyMarkdown(int& failures) -> void {
  const RuntimeTestPaths paths =
      BuildTempTestPaths("time_tracer_report_config_loader_invalid_test");
  const std::filesystem::path kInvalidReportPath =
      paths.test_root / "day_invalid.toml";

  RemoveTree(paths.test_root);
  std::filesystem::create_directories(paths.test_root);

  {
    std::ofstream file(kInvalidReportPath);
    file << "title_prefix = \"Daily Report for\"\n";
  }

  bool threw = false;
  std::string message;
  try {
    static_cast<void>(
        ReportConfigLoader::LoadDailyMdConfig(kInvalidReportPath));
  } catch (const std::exception& exception) {
    threw = true;
    message = exception.what();
  }

  if (!threw) {
    ++failures;
    std::cerr
        << "[FAIL] LoadDailyMdConfig should fail for invalid report config.\n";
  } else if (!Contains(message, "Invalid report config [") ||
             !Contains(message, "date_label")) {
    ++failures;
    std::cerr
        << "[FAIL] Invalid report config error should include context and "
           "missing key, actual: "
        << message << '\n';
  }

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunCoreConfigValidationTests(int& failures) -> void {
  TestAndroidRuntimeRejectsInvalidConverterConfig(failures);
  TestReportConfigLoaderRejectsInvalidDailyMarkdown(failures);
}

}  // namespace android_runtime_tests
