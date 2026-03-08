import tracer.core.infrastructure;
import tracer.core.infrastructure.config.loader.converter_config_loader;
import tracer.core.infrastructure.config.loader.report_config_loader;
import tracer.core.infrastructure.config.loader.toml_loader_utils;
import tracer.core.infrastructure.config.internal.config_detail_loader;
import tracer.core.infrastructure.config.internal.config_parser_utils;

#include <toml++/toml.h>

#include "application/ports/logger.hpp"
#include "domain/types/converter_config.hpp"
#include "infrastructure/config/models/app_config.hpp"
#include "infrastructure/config/models/report_config_models.hpp"

auto main() -> int {
  tracer::core::infrastructure::modlogging::ConsoleLogger logger;
  logger.Log(tracer_core::application::ports::LogSeverity::kInfo,
             "phase5 infrastructure module smoke logger");

  tracer::core::infrastructure::modlogging::FileErrorReportWriter writer(
      "temp/phase5_infra_module_smoke_error.log");
  if (!writer.Append("phase5 infrastructure module smoke\n")) {
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
  tracer::core::infrastructure::modconfig::StaticConverterConfigProvider
      provider(config);
  const auto loaded = provider.LoadConverterConfig();
  if (loaded.remark_prefix.compare("#") != 0) {
    return 4;
  }

  const auto load_converter_config =
      &tracer::core::infrastructure::modconfig::ConverterConfigLoader::
          LoadFromFile;
  const auto load_daily_md_config =
      &tracer::core::infrastructure::modconfig::ReportConfigLoader::
          LoadDailyMdConfig;
  const auto read_toml =
      &tracer::core::infrastructure::modconfig::loader::ReadToml;
  const auto load_detailed_reports =
      &tracer::core::infrastructure::modconfig::internal::LoadDetailedReports;
  const auto parse_system_settings =
      &tracer::core::infrastructure::modconfig::internal::
          ParseSystemSettings;
  (void)load_converter_config;
  (void)load_daily_md_config;
  (void)read_toml;
  (void)load_detailed_reports;
  (void)parse_system_settings;

  return 0;
}
