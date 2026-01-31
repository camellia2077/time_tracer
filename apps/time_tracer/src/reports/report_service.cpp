// reports/report_service.cpp
#include "report_service.hpp"

#include "reports/shared/generators/base_generator.hpp"

// [修改] 指向新的 data 模块路径
#include "reports/data/model/daily_report_data.hpp"
#include "reports/data/model/monthly_report_data.hpp"
#include "reports/data/model/period_report_data.hpp"
#include "reports/data/queriers/daily/day_querier.hpp"
#include "reports/data/queriers/monthly/month_querier.hpp"
#include "reports/data/queriers/period/period_querier.hpp"
#include "reports/services/daily_report_service.hpp"
#include "reports/services/monthly_report_service.hpp"
#include "reports/services/period_report_service.hpp"

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
                                       "MonthMdFormatter");
  MonthFactory::register_dll_formatter(ReportFormat::Typ, "MonthTypFormatter");
  MonthFactory::register_dll_formatter(ReportFormat::LaTeX,
                                       "MonthTexFormatter");

  // --- 注册周期报告格式 (PeriodReportData) ---
  using PeriodFactory = GenericFormatterFactory<PeriodReportData>;
  PeriodFactory::register_dll_formatter(ReportFormat::Markdown,
                                        "PeriodMdFormatter");
  PeriodFactory::register_dll_formatter(ReportFormat::LaTeX,
                                        "PeriodTexFormatter");
  PeriodFactory::register_dll_formatter(ReportFormat::Typ,
                                        "PeriodTypFormatter");

  return true;
}

// 静态变量初始化会触发上面的函数执行
bool is_registered = RegisterFormatters();
}  // namespace

ReportService::ReportService(sqlite3* db, const AppConfig& config)
    : db_(db), app_config_(config) {}

auto ReportService::run_daily_query(const std::string& date,
                                    ReportFormat format) const -> std::string {
  BaseGenerator<DailyReportData, DayQuerier, const std::string&> generator(
      db_, app_config_);
  return generator.generate_report(date, format);
}

auto ReportService::run_monthly_query(const std::string& month,
                                      ReportFormat format) const
    -> std::string {
  BaseGenerator<MonthlyReportData, MonthQuerier, const std::string&> generator(
      db_, app_config_);
  return generator.generate_report(month, format);
}

auto ReportService::run_period_query(int days, ReportFormat format) const
    -> std::string {
  BaseGenerator<PeriodReportData, PeriodQuerier, int> generator(db_,
                                                                app_config_);
  return generator.generate_report(days, format);
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
  PeriodReportService generator(db_, app_config_);
  return generator.generate_reports(days_list, format);
}