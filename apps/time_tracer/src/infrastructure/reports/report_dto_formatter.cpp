// infrastructure/reports/report_dto_formatter.cpp
#include "infrastructure/reports/report_dto_formatter.hpp"

#include <utility>

#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"

namespace infrastructure::reports {

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

}  // namespace infrastructure::reports
