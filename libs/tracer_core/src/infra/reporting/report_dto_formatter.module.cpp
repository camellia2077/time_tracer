#include <map>
#include <memory>
#include <string>
#include <utility>

#include "infra/reporting/report_dto_formatter.hpp"
#include "application/ports/reporting/i_report_dto_formatter.hpp"
#include "infra/config/models/report_catalog.hpp"
#include "infra/reporting/shared/factories/generic_formatter_factory.hpp"
#include "infra/reporting/shared/interfaces/i_report_formatter.hpp"

namespace tracer::core::infrastructure::reports {

ReportDtoFormatter::ReportDtoFormatter(const ReportCatalog& report_catalog)
    : report_catalog_(report_catalog) {}

auto ReportDtoFormatter::FormatDaily(const DailyReportData& report,
                                     ReportFormat format) -> std::string {
  return FormatWithCache(report, format, daily_cache_);
}

auto ReportDtoFormatter::FormatMonthly(const MonthlyReportData& report,
                                       ReportFormat format) -> std::string {
  return FormatWithCache(report, format, monthly_cache_);
}

auto ReportDtoFormatter::FormatPeriod(const PeriodReportData& report,
                                      ReportFormat format) -> std::string {
  return FormatWithCache(report, format, period_cache_);
}

auto ReportDtoFormatter::FormatWeekly(const WeeklyReportData& report,
                                      ReportFormat format) -> std::string {
  return FormatWithCache(report, format, weekly_cache_);
}

auto ReportDtoFormatter::FormatYearly(const YearlyReportData& report,
                                      ReportFormat format) -> std::string {
  return FormatWithCache(report, format, yearly_cache_);
}

template <typename ReportDataType>
auto ReportDtoFormatter::FormatWithCache(
    const ReportDataType& report, ReportFormat format,
    std::map<ReportFormat, std::unique_ptr<IReportFormatter<ReportDataType>>>&
        cache) -> std::string {
  auto formatter_iter = cache.find(format);
  if (formatter_iter == cache.end()) {
    auto formatter = GenericFormatterFactory<ReportDataType>::Create(
        format, report_catalog_);
    formatter_iter = cache.emplace(format, std::move(formatter)).first;
  }
  return formatter_iter->second->FormatReport(report);
}

}  // namespace tracer::core::infrastructure::reports
