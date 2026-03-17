#include "infrastructure/sqlite_fwd.hpp"

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "infrastructure/reports/report_service.hpp"
#include "application/interfaces/i_report_query_service.hpp"
#include "application/ports/i_platform_clock.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/data/queriers/daily/daily_querier.hpp"
#include "infrastructure/reports/data/queriers/monthly/monthly_querier.hpp"
#include "infrastructure/reports/data/queriers/period/batch_period_data_fetcher.hpp"
#include "infrastructure/reports/data/queriers/period/period_querier.hpp"
#include "infrastructure/reports/data/queriers/weekly/weekly_querier.hpp"
#include "infrastructure/reports/data/queriers/yearly/yearly_querier.hpp"
#include "infrastructure/reports/services/batch_export_helpers.hpp"
#include "infrastructure/reports/shared/generators/base_generator.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

import tracer.core.domain.reports.models.daily_report_data;
import tracer.core.infrastructure.reports.querying.services;

namespace modreports = tracer::core::domain::modreports;
namespace tracer::core::infrastructure::reports {

ReportService::ReportService(
    sqlite3* sqlite_db, const ReportCatalog& catalog,
    std::shared_ptr<tracer_core::application::ports::IPlatformClock>
        platform_clock)
    : db_(sqlite_db),
      report_catalog_(catalog),
      platform_clock_(std::move(platform_clock)) {
  if (!platform_clock_) {
    throw std::invalid_argument(
        "ReportService platform clock must not be null.");
  }
}

auto ReportService::RunDailyQuery(std::string_view date,
                                  ReportFormat format) const -> std::string {
  BaseGenerator<modreports::DailyReportData, DayQuerier, std::string_view>
      generator(
      db_, report_catalog_);
  return generator.GenerateReport(date, format);
}

auto ReportService::RunMonthlyQuery(std::string_view year_month_str,
                                    ReportFormat format) const -> std::string {
  BaseGenerator<MonthlyReportData, MonthQuerier, std::string_view> generator(
      db_, report_catalog_);
  return generator.GenerateReport(year_month_str, format);
}

auto ReportService::RunPeriodQuery(int days, ReportFormat format) const
    -> std::string {
  PeriodQuerier querier(db_, days, *platform_clock_);
  PeriodReportData report_data = querier.FetchData();
  auto& formatter = GetOrCreatePeriodFormatter(format);
  return formatter.FormatReport(report_data);
}

auto ReportService::RunWeeklyQuery(std::string_view iso_week_str,
                                   ReportFormat format) const -> std::string {
  BaseGenerator<WeeklyReportData, WeekQuerier, std::string_view> generator(
      db_, report_catalog_);
  return generator.GenerateReport(iso_week_str, format);
}

auto ReportService::RunYearlyQuery(std::string_view year_str,
                                   ReportFormat format) const -> std::string {
  BaseGenerator<YearlyReportData, YearQuerier, std::string_view> generator(
      db_, report_catalog_);
  return generator.GenerateReport(year_str, format);
}

auto ReportService::RunExportAllDailyReportsQuery(ReportFormat format) const
    -> FormattedGroupedReports {
  services::DailyReportService generator(db_, report_catalog_);
  return generator.GenerateAllReports(format);
}

auto ReportService::RunExportAllMonthlyReportsQuery(ReportFormat format) const
    -> FormattedMonthlyReports {
  services::MonthlyReportService generator(db_, report_catalog_);
  return generator.GenerateReports(format);
}

auto ReportService::RunExportAllPeriodReportsQuery(
    const std::vector<int>& days_list, ReportFormat format) const
    -> FormattedPeriodReports {
  FormattedPeriodReports reports;
  if (days_list.empty()) {
    return reports;
  }

  ProjectNameCache name_cache =
      ::reports::services::CreateProjectNameCache(db_);

  BatchPeriodDataFetcher fetcher(db_, *platform_clock_);
  std::map<int, PeriodReportData> all_data = fetcher.FetchAllData(days_list);

  auto formatter = GenericFormatterFactory<PeriodReportData>::Create(
      format, report_catalog_);

  ::reports::services::FormatReportMap(
      all_data, formatter, name_cache,
      [&](int days, const std::string& formatted_report) -> void {
        if (days > 0) {
          reports[days] = formatted_report;
        }
      });

  return reports;
}

auto ReportService::RunExportAllWeeklyReportsQuery(ReportFormat format) const
    -> FormattedWeeklyReports {
  services::WeeklyReportService generator(db_, report_catalog_);
  return generator.GenerateReports(format);
}

auto ReportService::RunExportAllYearlyReportsQuery(ReportFormat format) const
    -> FormattedYearlyReports {
  services::YearlyReportService generator(db_, report_catalog_);
  return generator.GenerateReports(format);
}

auto ReportService::GetOrCreatePeriodFormatter(ReportFormat format) const
    -> IReportFormatter<PeriodReportData>& {
  if (auto formatter_iter = period_formatter_cache_.find(format);
      formatter_iter != period_formatter_cache_.end()) {
    return *(formatter_iter->second);
  }

  auto formatter = GenericFormatterFactory<PeriodReportData>::Create(
      format, report_catalog_);
  auto [inserted_iter, inserted] =
      period_formatter_cache_.emplace(format, std::move(formatter));
  if (!inserted || !(inserted_iter->second)) {
    throw std::runtime_error(
        "Failed to cache period formatter for selected report format.");
  }

  return *(inserted_iter->second);
}

}  // namespace tracer::core::infrastructure::reports
