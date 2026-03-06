#include "infrastructure/config/static_converter_config_provider.hpp"
#include "infrastructure/logging/console_diagnostics_sink.hpp"
#include "infrastructure/logging/console_logger.hpp"
#include "infrastructure/logging/file_error_report_writer.hpp"
#include "infrastructure/logging/validation_issue_reporter.hpp"
#include "infrastructure/platform/android/android_platform_clock.hpp"
#include "infrastructure/platform/windows/windows_platform_clock.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace {

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

void TestLegacyLoggingHeaders(int& failures) {
  using infrastructure::logging::ConsoleDiagnosticsSink;
  using infrastructure::logging::ConsoleLogger;
  using infrastructure::logging::FileErrorReportWriter;
  using infrastructure::logging::ValidationIssueReporter;

  Expect(std::is_class_v<ConsoleLogger>,
         "Legacy ConsoleLogger header path should remain visible.", failures);
  Expect(std::is_class_v<ConsoleDiagnosticsSink>,
         "Legacy ConsoleDiagnosticsSink header path should remain visible.",
         failures);
  Expect(std::is_class_v<FileErrorReportWriter>,
         "Legacy FileErrorReportWriter header path should remain visible.",
         failures);
  Expect(std::is_class_v<ValidationIssueReporter>,
         "Legacy ValidationIssueReporter header path should remain visible.",
         failures);

  ConsoleLogger logger;
  logger.Log(tracer_core::application::ports::LogSeverity::kInfo,
             "phase5 infrastructure legacy header logger");

  const std::filesystem::path report_path =
      std::filesystem::path("temp") / "phase5_infra_legacy_header_error.log";
  FileErrorReportWriter writer(report_path);
  const bool appended = writer.Append("phase5 infrastructure legacy smoke\n");
  Expect(appended,
         "Legacy FileErrorReportWriter should append to report file.",
         failures);
}

void TestLegacyPlatformHeaders(int& failures) {
  using infrastructure::platform::AndroidPlatformClock;
  using infrastructure::platform::WindowsPlatformClock;

  Expect(std::is_class_v<WindowsPlatformClock>,
         "Legacy WindowsPlatformClock header path should remain visible.",
         failures);
  Expect(std::is_class_v<AndroidPlatformClock>,
         "Legacy AndroidPlatformClock header path should remain visible.",
         failures);

  WindowsPlatformClock windows_clock;
  const std::string windows_date = windows_clock.TodayLocalDateIso();
  const int windows_offset = windows_clock.LocalUtcOffsetMinutes();
  Expect(windows_date.size() == 10U,
         "Legacy WindowsPlatformClock date should be YYYY-MM-DD.", failures);
  Expect(windows_offset >= -24 * 60 && windows_offset <= 24 * 60,
         "Legacy WindowsPlatformClock UTC offset should be in valid range.",
         failures);

  AndroidPlatformClock android_clock;
  const std::string android_date = android_clock.TodayLocalDateIso();
  const int android_offset = android_clock.LocalUtcOffsetMinutes();
  Expect(android_date.size() == 10U,
         "Legacy AndroidPlatformClock date should be YYYY-MM-DD.", failures);
  Expect(android_offset >= -24 * 60 && android_offset <= 24 * 60,
         "Legacy AndroidPlatformClock UTC offset should be in valid range.",
         failures);
}

void TestLegacyConfigHeaders(int& failures) {
  using infrastructure::config::StaticConverterConfigProvider;
  Expect(
      std::is_class_v<StaticConverterConfigProvider>,
      "Legacy StaticConverterConfigProvider header path should remain visible.",
      failures);

  ConverterConfig config;
  config.remark_prefix = "#";
  config.text_mapping["sleep"] = "sleep_night";
  StaticConverterConfigProvider provider(config);
  const ConverterConfig loaded = provider.LoadConverterConfig();
  Expect(loaded.text_mapping.contains("sleep"),
         "Legacy StaticConverterConfigProvider text_mapping mismatch.",
         failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestLegacyLoggingHeaders(failures);
  TestLegacyPlatformHeaders(failures);
  TestLegacyConfigHeaders(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_infrastructure_legacy_headers_compat_tests\n";
    return 0;
  }

  std::cerr
      << "[FAIL] tracer_core_infrastructure_legacy_headers_compat_tests failures: "
      << failures << '\n';
  return 1;
}
