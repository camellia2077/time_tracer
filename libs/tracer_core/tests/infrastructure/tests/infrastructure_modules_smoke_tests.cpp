import tracer.core.infrastructure;

#include "application/ports/logger.hpp"
#include "domain/types/converter_config.hpp"

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

  return 0;
}
