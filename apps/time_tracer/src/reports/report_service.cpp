// reports/report_service.cpp
#include "report_service.hpp"

#include "reports/shared/generators/base_generator.hpp"

// [修改] 指向新的 data 模块路径
#include "reports/data/cache/project_name_cache.hpp"
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
#include "reports/data/utils/project_tree_builder.hpp"
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
  DayFactory::register_dll_formatter(ReportFormat::Markdown, "DayMdFormatter");
  DayFactory::register_dll_formatter(ReportFormat::Typ, "DayTypFormatter");
  DayFactory::register_dll_formatter(ReportFormat::LaTeX, "DayTexFormatter");

  // --- 注册月报格式 (MonthlyReportData) ---
  using MonthFactory = GenericFormatterFactory<MonthlyReportData>;
  MonthFactory::register_dll_formatter(ReportFormat::Markdown,
                                       "RangeMdFormatter");
  MonthFactory::register_dll_formatter(ReportFormat::Typ, "RangeTypFormatter");
  MonthFactory::register_dll_formatter(ReportFormat::LaTeX,
                                       "RangeTexFormatter");

  // --- 注册周期报告格式 (PeriodReportData) ---
  using PeriodFactory = GenericFormatterFactory<PeriodReportData>;
  PeriodFactory::register_dll_formatter(ReportFormat::Markdown,
                                        "RangeMdFormatter");
  PeriodFactory::register_dll_formatter(ReportFormat::LaTeX,
                                        "RangeTexFormatter");
  PeriodFactory::register_dll_formatter(ReportFormat::Typ, "RangeTypFormatter");

  // --- Register Weekly Report Formatters (WeeklyReportData) ---
  using WeekFactory = GenericFormatterFactory<WeeklyReportData>;
  WeekFactory::register_dll_formatter(ReportFormat::Markdown,
                                      "RangeMdFormatter");
  WeekFactory::register_dll_formatter(ReportFormat::LaTeX, "RangeTexFormatter");
  WeekFactory::register_dll_formatter(ReportFormat::Typ, "RangeTypFormatter");

  // --- Register Yearly Report Formatters (YearlyReportData) ---
  using YearFactory = GenericFormatterFactory<YearlyReportData>;
  YearFactory::register_dll_formatter(ReportFormat::Markdown,
                                      "RangeMdFormatter");
  YearFactory::register_dll_formatter(ReportFormat::LaTeX, "RangeTexFormatter");
  YearFactory::register_dll_formatter(ReportFormat::Typ, "RangeTypFormatter");

  return true;
}

// 静态变量初始化会触发上面的函数执行
bool is_registered = RegisterFormatters();
}  // namespace

ReportService::ReportService(sqlite3* sqlite_db, const AppConfig& config)
    : db_(sqlite_db), app_config_(config) {}

auto ReportService::run_daily_query(const std::string& date,
                                    ReportFormat format) const -> std::string {
  BaseGenerator<DailyReportData, DayQuerier, const std::string&> generator(
      db_, app_config_);
  return generator.generate_report(date, format);
}

auto ReportService::run_monthly_query(const std::string& year_month_str,
                                      ReportFormat format) const
    -> std::string {
  BaseGenerator<MonthlyReportData, MonthQuerier, const std::string&> generator(
      db_, app_config_);
  return generator.generate_report(year_month_str, format);
}

auto ReportService::run_period_query(int days, ReportFormat format) const
    -> std::string {
  BaseGenerator<PeriodReportData, PeriodQuerier, int> generator(db_,
                                                                app_config_);
  return generator.generate_report(days, format);
}

auto ReportService::run_weekly_query(const std::string& iso_week_str,
                                     ReportFormat format) const -> std::string {
  BaseGenerator<WeeklyReportData, WeekQuerier, const std::string&> generator(
      db_, app_config_);
  return generator.generate_report(iso_week_str, format);
}

auto ReportService::run_yearly_query(const std::string& year_str,
                                     ReportFormat format) const -> std::string {
  BaseGenerator<YearlyReportData, YearQuerier, const std::string&> generator(
      db_, app_config_);
  return generator.generate_report(year_str, format);
}

auto ReportService::run_export_all_daily_reports_query(
    ReportFormat format) const -> FormattedGroupedReports {
  DailyReportService generator(db_, app_config_);
  return generator.generate_all_reports(format);
}

auto ReportService::run_export_all_monthly_reports_query(
    ReportFormat format) const -> FormattedMonthlyReports {
  MonthlyReportService generator(db_, app_config_);
  return generator.generate_reports(format);
}

auto ReportService::run_export_all_period_reports_query(
    const std::vector<int>& days_list, ReportFormat format) const
    -> FormattedPeriodReports {
  FormattedPeriodReports reports;
  if (days_list.empty()) {
    return reports;
  }

  auto& name_cache = ProjectNameCache::instance();
  name_cache.ensure_loaded(db_);

  BatchPeriodDataFetcher fetcher(db_);
  std::map<int, PeriodReportData> all_data = fetcher.fetch_all_data(days_list);

  auto formatter =
      GenericFormatterFactory<PeriodReportData>::create(format, app_config_);

  for (auto& [days, data] : all_data) {
    if (days > 0 && data.total_duration > 0) {
      build_project_tree_from_ids(data.project_tree, data.project_stats,
                                  name_cache);
      std::string formatted_report = formatter->format_report(data);
      reports[days] = formatted_report;
    }
  }

  return reports;
}

auto ReportService::run_export_all_weekly_reports_query(
    ReportFormat format) const -> FormattedWeeklyReports {
  WeeklyReportService generator(db_, app_config_);
  return generator.generate_reports(format);
}

auto ReportService::run_export_all_yearly_reports_query(
    ReportFormat format) const -> FormattedYearlyReports {
  YearlyReportService generator(db_, app_config_);
  return generator.generate_reports(format);
}
