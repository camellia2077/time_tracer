// application/reporting/generator/report_generator.cpp
#include "application/reporting/generator/report_generator.hpp"

#include "reports/report_service.hpp"

ReportGenerator::ReportGenerator(sqlite3* sqlite_db, const AppConfig& config) {
  query_handler_ = std::make_unique<ReportService>(sqlite_db, config);
}

ReportGenerator::~ReportGenerator() = default;

auto ReportGenerator::GenerateDailyReport(std::string_view date,
                                         ReportFormat format) -> std::string {
  return query_handler_->RunDailyQuery(date, format);
}

auto ReportGenerator::GenerateMonthlyReport(std::string_view month,
                                           ReportFormat format) -> std::string {
  return query_handler_->RunMonthlyQuery(month, format);
}

auto ReportGenerator::GeneratePeriodReport(int days, ReportFormat format)
    -> std::string {
  return query_handler_->RunPeriodQuery(days, format);
}

auto ReportGenerator::GenerateWeeklyReport(std::string_view iso_week,
                                          ReportFormat format) -> std::string {
  return query_handler_->RunWeeklyQuery(iso_week, format);
}

auto ReportGenerator::GenerateYearlyReport(std::string_view year,
                                          ReportFormat format) -> std::string {
  return query_handler_->RunYearlyQuery(year, format);
}

auto ReportGenerator::GenerateAllDailyReports(ReportFormat format)
    -> FormattedGroupedReports {
  return query_handler_->RunExportAllDailyReportsQuery(format);
}

auto ReportGenerator::GenerateAllMonthlyReports(ReportFormat format)
    -> FormattedMonthlyReports {
  return query_handler_->RunExportAllMonthlyReportsQuery(format);
}

auto ReportGenerator::GenerateAllPeriodReports(
    const std::vector<int>& days_list, ReportFormat format)
    -> FormattedPeriodReports {
  return query_handler_->RunExportAllPeriodReportsQuery(days_list, format);
}

auto ReportGenerator::GenerateAllWeeklyReports(ReportFormat format)
    -> FormattedWeeklyReports {
  return query_handler_->RunExportAllWeeklyReportsQuery(format);
}

auto ReportGenerator::GenerateAllYearlyReports(ReportFormat format)
    -> FormattedYearlyReports {
  return query_handler_->RunExportAllYearlyReportsQuery(format);
}
