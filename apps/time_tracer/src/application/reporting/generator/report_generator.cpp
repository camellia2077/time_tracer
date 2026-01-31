// application/reporting/generator/report_generator.cpp
#include "report_generator.hpp"

#include "reports/report_service.hpp"

ReportGenerator::ReportGenerator(sqlite3* db, const AppConfig& config) {
  query_handler_ = std::make_unique<ReportService>(db, config);
}

ReportGenerator::~ReportGenerator() = default;

auto ReportGenerator::generate_daily_report(const std::string& date,
                                            ReportFormat format)
    -> std::string {
  return query_handler_->run_daily_query(date, format);
}

auto ReportGenerator::generate_monthly_report(const std::string& month,
                                              ReportFormat format)
    -> std::string {
  return query_handler_->run_monthly_query(month, format);
}

auto ReportGenerator::generate_period_report(int days, ReportFormat format)
    -> std::string {
  return query_handler_->run_period_query(days, format);
}

auto ReportGenerator::generate_all_daily_reports(ReportFormat format)
    -> FormattedGroupedReports {
  return query_handler_->run_export_all_daily_reports_query(format);
}

auto ReportGenerator::generate_all_monthly_reports(ReportFormat format)
    -> FormattedMonthlyReports {
  return query_handler_->run_export_all_monthly_reports_query(format);
}

auto ReportGenerator::generate_all_period_reports(
    const std::vector<int>& days_list, ReportFormat format)
    -> FormattedPeriodReports {
  return query_handler_->run_export_all_period_reports_query(days_list, format);
}