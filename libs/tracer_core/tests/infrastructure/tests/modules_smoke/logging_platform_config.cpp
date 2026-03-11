import tracer.core.infrastructure;
import tracer.core.infrastructure.config;

#include <toml++/toml.h>

#include "infrastructure/tests/modules_smoke/support.hpp"

auto RunInfrastructureModuleLoggingPlatformConfigSmoke() -> int {
  tracer::core::infrastructure::logging::ConsoleLogger logger;
  logger.Log(tracer_core::application::ports::LogSeverity::kInfo,
             "phase6 infrastructure module smoke logger");

  tracer::core::infrastructure::logging::ConsoleDiagnosticsSink sink;
  (void)sink;

  tracer::core::infrastructure::logging::ValidationIssueReporter reporter;
  (void)reporter;

  tracer::core::infrastructure::logging::FileErrorReportWriter writer(
      "temp/phase6_infra_module_smoke_error.log");
  if (!writer.Append("phase6 infrastructure module smoke\n")) {
    return 1;
  }

  tracer::core::infrastructure::modplatform::WindowsPlatformClock windows_clock;
  const auto windows_date = windows_clock.TodayLocalDateIso();
  if (windows_date.size() != 10U) {
    return 2;
  }

  tracer::core::infrastructure::modplatform::AndroidPlatformClock android_clock;
  const auto android_date = android_clock.TodayLocalDateIso();
  if (android_date.size() != 10U) {
    return 3;
  }

  ConverterConfig config;
  config.remark_prefix = "#";
  tracer::core::infrastructure::config::StaticConverterConfigProvider provider(
      config);
  const auto loaded = provider.LoadConverterConfig();
  if (loaded.remark_prefix.compare("#") != 0) {
    return 4;
  }

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
      &tracer::core::infrastructure::config::internal::
          ParseSystemSettings;
  const auto load_configuration =
      &tracer::core::infrastructure::config::ConfigLoader::LoadConfiguration;
  const auto load_file_provider =
      &tracer::core::infrastructure::config::FileConverterConfigProvider::
          LoadConverterConfig;
  (void)load_converter_config;
  (void)load_daily_md_config;
  (void)read_toml;
  (void)load_detailed_reports;
  (void)parse_system_settings;
  (void)load_configuration;
  (void)load_file_provider;

  std::error_code cleanup_error;

  const std::filesystem::path kConfigSmokeDir =
      std::filesystem::path("temp") / "phase4_config_infra_module_smoke";
  const std::filesystem::path kCopiedConfigRoot = kConfigSmokeDir / "config";
  const std::filesystem::path kFakeExePath =
      kConfigSmokeDir / "bin" / "tracer_core_smoke.exe";
  std::filesystem::remove_all(kConfigSmokeDir, cleanup_error);
  std::filesystem::create_directories(kConfigSmokeDir);
  std::filesystem::copy(std::filesystem::path("assets") / "tracer_core" /
                            "config",
                        kCopiedConfigRoot,
                        std::filesystem::copy_options::recursive);
  WriteSmokeFile(kFakeExePath, "smoke");

  const std::filesystem::path kDailyMarkdownConfig =
      kCopiedConfigRoot / "reports" / "markdown" / "day.toml";
  const toml::table kDailyMarkdownTable =
      tracer::core::infrastructure::config::loader::ReadToml(
          kDailyMarkdownConfig);
  if (!kDailyMarkdownTable.contains("date_label")) {
    return 400;
  }

  const auto kDailyMarkdown =
      tracer::core::infrastructure::config::ReportConfigLoader::
          LoadDailyMdConfig(kDailyMarkdownConfig);
  if (kDailyMarkdown.labels.date_label != "Date" ||
      kDailyMarkdown.statistics_items.empty()) {
    return 401;
  }

  tracer::core::infrastructure::config::FileConverterConfigProvider
      file_provider(
          kCopiedConfigRoot / "converter" / "interval_processor_config.toml",
          std::unordered_map<std::filesystem::path, std::filesystem::path>{});
  const ConverterConfig kLoadedFileConfig = file_provider.LoadConverterConfig();
  if (kLoadedFileConfig.remark_prefix != "r " ||
      !kLoadedFileConfig.text_mapping.contains("wake")) {
    return 402;
  }

  tracer::core::infrastructure::config::ConfigLoader config_loader(
      kFakeExePath.string());
  const AppConfig kLoadedAppConfig = config_loader.LoadConfiguration();
  if (kLoadedAppConfig.pipeline.interval_processor_config_path.filename() !=
          "interval_processor_config.toml" ||
      kLoadedAppConfig.loaded_reports.markdown.day.labels.date_label !=
          "Date") {
    return 403;
  }

  std::filesystem::remove_all(kConfigSmokeDir, cleanup_error);
  return 0;
}
