// reports/report_service.cpp
#include "reports/report_service.hpp"

#include "reports/shared/generators/base_generator.hpp"

// [修改] 指向新的 data 模块路径
#include "reports/data/model/daily_report_data.hpp"
#include "reports/data/model/monthly_report_data.hpp"
#include "reports/data/model/period_report_data.hpp"
#include "reports/data/model/weekly_report_data.hpp"
#include "reports/data/model/yearly_report_data.hpp"
#include "reports/data/queriers/daily/day_querier.hpp"
#include "reports/data/queriers/monthly/month_querier.hpp"
#include "reports/data/queriers/period/batch_period_data_fetcher.hpp"
#include "reports/data/queriers/period/period_querier.hpp"
#include "reports/data/queriers/weekly/week_querier.hpp"
#include "reports/data/queriers/yearly/year_querier.hpp"
#include "reports/services/batch_export_helpers.hpp"
#include "reports/services/daily_report_service.hpp"
#include "reports/services/monthly_report_service.hpp"
#include "reports/services/weekly_report_service.hpp"
#include "reports/services/yearly_report_service.hpp"

// [新增] 引入工厂头文件以便进行注册
#include "reports/shared/factories/generic_formatter_factory.hpp"

// [新增] 静态注册逻辑
// 使用匿名命名空间和立即执行的 lambda 来确保只注册一次
namespace {
auto RegisterFormatters() -> bool {
  // --- 注册日报格式 (DailyReportData) ---
  using DayFactory = GenericFormatterFactory<DailyReportData>;
  DayFactory::RegisterDllFormatter(ReportFormat::kMarkdown, "DayMdFormatter");
  DayFactory::RegisterDllFormatter(ReportFormat::kTyp, "DayTypFormatter");
  DayFactory::RegisterDllFormatter(ReportFormat::kLaTeX, "DayTexFormatter");

  // --- 注册月报格式 (MonthlyReportData) ---
  using MonthFactory = GenericFormatterFactory<MonthlyReportData>;
  MonthFactory::RegisterDllFormatter(ReportFormat::kMarkdown,
                                       "RangeMdFormatter");
  MonthFactory::RegisterDllFormatter(ReportFormat::kTyp, "RangeTypFormatter");
  MonthFactory::RegisterDllFormatter(ReportFormat::kLaTeX,
                                       "RangeTexFormatter");

  // --- 注册周期报告格式 (PeriodReportData) ---
  using PeriodFactory = GenericFormatterFactory<PeriodReportData>;
  PeriodFactory::RegisterDllFormatter(ReportFormat::kMarkdown,
                                        "RangeMdFormatter");
  PeriodFactory::RegisterDllFormatter(ReportFormat::kLaTeX,
                                        "RangeTexFormatter");
  PeriodFactory::RegisterDllFormatter(ReportFormat::kTyp, "RangeTypFormatter");

  // --- Register Weekly Report Formatters (WeeklyReportData) ---
  using WeekFactory = GenericFormatterFactory<WeeklyReportData>;
  WeekFactory::RegisterDllFormatter(ReportFormat::kMarkdown,
                                      "RangeMdFormatter");
  WeekFactory::RegisterDllFormatter(ReportFormat::kLaTeX, "RangeTexFormatter");
  WeekFactory::RegisterDllFormatter(ReportFormat::kTyp, "RangeTypFormatter");

  // --- Register Yearly Report Formatters (YearlyReportData) ---
  using YearFactory = GenericFormatterFactory<YearlyReportData>;
  YearFactory::RegisterDllFormatter(ReportFormat::kMarkdown,
                                      "RangeMdFormatter");
  YearFactory::RegisterDllFormatter(ReportFormat::kLaTeX, "RangeTexFormatter");
  YearFactory::RegisterDllFormatter(ReportFormat::kTyp, "RangeTypFormatter");


  return true;
}

// 静态变量初始化会触发上面的函数执行
bool is_registered = RegisterFormatters();
}  // namespace

ReportService::ReportService(sqlite3* sqlite_db, const AppConfig& config)
    : db_(sqlite_db), app_config_(config) {}

auto ReportService::RunDailyQuery(std::string_view date,
                                    ReportFormat format) const
    -> std::string {
  BaseGenerator<DailyReportData, DayQuerier, std::string_view> generator(
      db_, app_config_);
  return generator.GenerateReport(date, format);
}

auto ReportService::RunMonthlyQuery(std::string_view year_month_str,
                                       ReportFormat format) const
    -> std::string {
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
                                      ReportFormat format) const
    -> std::string {
  BaseGenerator<WeeklyReportData, WeekQuerier, std::string_view> generator(
      db_, app_config_);
  return generator.GenerateReport(iso_week_str, format);
}

auto ReportService::RunYearlyQuery(std::string_view year_str,
                                      ReportFormat format) const
    -> std::string {
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
