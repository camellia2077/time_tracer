// reports/report_service.cpp
#include "infrastructure/reports/report_service.hpp"

#include "infrastructure/reports/shared/generators/base_generator.hpp"

// [修改] 指向新的 data 模块路径
#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/monthly_report_data.hpp"
#include "domain/reports/models/period_report_data.hpp"
#include "domain/reports/models/weekly_report_data.hpp"
#include "domain/reports/models/yearly_report_data.hpp"
#include "infrastructure/reports/data/queriers/daily/day_querier.hpp"
#include "infrastructure/reports/data/queriers/monthly/month_querier.hpp"
#include "infrastructure/reports/data/queriers/period/batch_period_data_fetcher.hpp"
#include "infrastructure/reports/data/queriers/period/period_querier.hpp"
#include "infrastructure/reports/data/queriers/weekly/week_querier.hpp"
#include "infrastructure/reports/data/queriers/yearly/year_querier.hpp"
#include "infrastructure/reports/services/batch_export_helpers.hpp"
#include "infrastructure/reports/services/daily_report_service.hpp"
#include "infrastructure/reports/services/monthly_report_service.hpp"
#include "infrastructure/reports/services/weekly_report_service.hpp"
#include "infrastructure/reports/services/yearly_report_service.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"

ReportService::ReportService(sqlite3* sqlite_db, const AppConfig& config)
    : db_(sqlite_db), app_config_(config) {}

auto ReportService::RunDailyQuery(std::string_view date,
                                  ReportFormat format) const -> std::string {
  BaseGenerator<DailyReportData, DayQuerier, std::string_view> generator(
      db_, app_config_);
  return generator.GenerateReport(date, format);
}

auto ReportService::RunMonthlyQuery(std::string_view year_month_str,
                                    ReportFormat format) const -> std::string {
  BaseGenerator<MonthlyReportData, MonthQuerier, std::string_view> generator(
      db_, app_config_);
  return generator.GenerateReport(year_month_str, format);
}

auto ReportService::RunPeriodQuery(int days, ReportFormat format) const
    -> std::string {
  BaseGenerator<PeriodReportData, PeriodQuerier, int> generator(db_,
                                                                app_config_);
  return generator.GenerateReport(days, format);
}

auto ReportService::RunWeeklyQuery(std::string_view iso_week_str,
                                   ReportFormat format) const -> std::string {
  BaseGenerator<WeeklyReportData, WeekQuerier, std::string_view> generator(
      db_, app_config_);
  return generator.GenerateReport(iso_week_str, format);
}

auto ReportService::RunYearlyQuery(std::string_view year_str,
                                   ReportFormat format) const -> std::string {
  BaseGenerator<YearlyReportData, YearQuerier, std::string_view> generator(
      db_, app_config_);
  return generator.GenerateReport(year_str, format);
}

auto ReportService::RunExportAllDailyReportsQuery(ReportFormat format) const
    -> FormattedGroupedReports {
  DailyReportService generator(db_, app_config_);
  return generator.generate_all_reports(format);
}

auto ReportService::RunExportAllMonthlyReportsQuery(ReportFormat format) const
    -> FormattedMonthlyReports {
  MonthlyReportService generator(db_, app_config_);
  return generator.GenerateReports(format);
}

auto ReportService::RunExportAllPeriodReportsQuery(
    const std::vector<int>& days_list, ReportFormat format) const
    -> FormattedPeriodReports {
  FormattedPeriodReports reports;
  if (days_list.empty()) {
    return reports;
  }

  auto& name_cache = reports::services::EnsureProjectNameCache(db_);

  BatchPeriodDataFetcher fetcher(db_);
  std::map<int, PeriodReportData> all_data = fetcher.FetchAllData(days_list);

  auto formatter =
      GenericFormatterFactory<PeriodReportData>::Create(format, app_config_);

  reports::services::FormatReportMap(
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
  WeeklyReportService generator(db_, app_config_);
  return generator.GenerateReports(format);
}

auto ReportService::RunExportAllYearlyReportsQuery(ReportFormat format) const
    -> FormattedYearlyReports {
  YearlyReportService generator(db_, app_config_);
  return generator.GenerateReports(format);
}
