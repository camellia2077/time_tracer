// infrastructure/reports/formatter_registry.cpp
#include "infrastructure/reports/formatter_registry.hpp"

#include <string>
#include <string_view>

#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/period_report_models.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/reports/plugin_manifest.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"

namespace {
using reports::plugin_manifest::FormatterPluginBinding;
using reports::plugin_manifest::ReportKind;

template <typename ReportDataType>
void RegisterBinding(ReportFormat format, std::string_view plugin_name) {
  GenericFormatterFactory<ReportDataType>::RegisterDllFormatter(
      format, std::string(plugin_name));
}

void RegisterBinding(const FormatterPluginBinding& binding) {
  switch (binding.report_kind) {
    case ReportKind::kDay:
      RegisterBinding<DailyReportData>(binding.format, binding.plugin_name);
      break;
    case ReportKind::kMonth:
      RegisterBinding<MonthlyReportData>(binding.format, binding.plugin_name);
      break;
    case ReportKind::kPeriod:
      RegisterBinding<PeriodReportData>(binding.format, binding.plugin_name);
      break;
    case ReportKind::kWeek:
      RegisterBinding<WeeklyReportData>(binding.format, binding.plugin_name);
      break;
    case ReportKind::kYear:
      RegisterBinding<YearlyReportData>(binding.format, binding.plugin_name);
      break;
  }
}
}  // namespace

namespace reports {
void RegisterReportFormatters() {
  static const bool kIsRegistered = []() -> bool {
    for (const auto& binding : plugin_manifest::GetFormatterPluginBindings()) {
      RegisterBinding(binding);
    }
    return true;
  }();

  (void)kIsRegistered;
}
}  // namespace reports
