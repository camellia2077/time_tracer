#include "infrastructure/tests/legacy_headers_compat/support.hpp"

namespace {

using CanonicalConsoleDiagnosticsSink =
    tracer::core::infrastructure::logging::ConsoleDiagnosticsSink;
using CanonicalConsoleLogger =
    tracer::core::infrastructure::logging::ConsoleLogger;
using CanonicalFileErrorReportWriter =
    tracer::core::infrastructure::logging::FileErrorReportWriter;
using CanonicalValidationIssueReporter =
    tracer::core::infrastructure::logging::ValidationIssueReporter;

}  // namespace

auto TestLegacyLoggingHeaders(int& failures) -> void {
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

  Expect(std::is_class_v<CanonicalConsoleLogger>,
         "Canonical ConsoleLogger header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalConsoleDiagnosticsSink>,
         "Canonical ConsoleDiagnosticsSink header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalFileErrorReportWriter>,
         "Canonical FileErrorReportWriter header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalValidationIssueReporter>,
         "Canonical ValidationIssueReporter header contract should be visible.",
         failures);
}

auto TestLegacyPlatformHeaders(int& failures) -> void {
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

auto TestLegacyConfigHeaders(int& failures) -> void {
  using CanonicalConfigLoader =
      tracer::core::infrastructure::config::ConfigLoader;
  using CanonicalFileConverterConfigProvider =
      tracer::core::infrastructure::config::FileConverterConfigProvider;
  using CanonicalStaticConverterConfigProvider =
      tracer::core::infrastructure::config::StaticConverterConfigProvider;

  using infrastructure::config::StaticConverterConfigProvider;
  Expect(
      std::is_class_v<StaticConverterConfigProvider>,
      "Legacy StaticConverterConfigProvider header path should remain visible.",
      failures);
  Expect(std::is_class_v<CanonicalConfigLoader>,
         "Canonical ConfigLoader header contract should remain visible.",
         failures);
  Expect(std::is_class_v<CanonicalFileConverterConfigProvider>,
         "Canonical FileConverterConfigProvider header contract should remain visible.",
         failures);
  Expect(std::is_class_v<CanonicalStaticConverterConfigProvider>,
         "Canonical StaticConverterConfigProvider header contract should remain visible.",
         failures);

  ConverterConfig config;
  config.remark_prefix = "#";
  config.text_mapping["sleep"] = "sleep_night";
  StaticConverterConfigProvider provider(config);
  const ConverterConfig loaded = provider.LoadConverterConfig();
  Expect(loaded.text_mapping.contains("sleep"),
         "Legacy StaticConverterConfigProvider text_mapping mismatch.",
         failures);

  const auto load_converter_config =
      &tracer::core::infrastructure::config::ConverterConfigLoader::
          LoadFromFile;
  const auto load_daily_md_config =
      &tracer::core::infrastructure::config::ReportConfigLoader::
          LoadDailyMdConfig;
  const auto read_toml =
      &tracer::core::infrastructure::config::loader::ReadToml;
  const auto load_detailed_reports =
      &tracer::core::infrastructure::config::internal::LoadDetailedReports;
  const auto parse_system_settings =
      &tracer::core::infrastructure::config::internal::ParseSystemSettings;
  (void)load_converter_config;
  (void)load_daily_md_config;
  (void)read_toml;
  (void)load_detailed_reports;
  (void)parse_system_settings;
}
