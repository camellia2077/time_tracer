// application/reporting/generator/report_generator.cpp
#include "application/reporting/generator/report_generator.hpp"

#include <utility>

ReportGenerator::ReportGenerator(
    std::unique_ptr<IReportQueryService> query_service)
    : query_service_(std::move(query_service)) {}

ReportGenerator::~ReportGenerator() = default;

auto ReportGenerator::GenerateDailyReport(std::string_view date,
                                          ReportFormat format) -> std::string {
  return query_service_->RunDailyQuery(date, format);
}

auto ReportGenerator::GenerateMonthlyReport(std::string_view month,
                                            ReportFormat format)
    -> std::string {
  return query_service_->RunMonthlyQuery(month, format);
}

auto ReportGenerator::GeneratePeriodReport(int days, ReportFormat format)
    -> std::string {
  return query_service_->RunPeriodQuery(days, format);
}

auto ReportGenerator::GenerateWeeklyReport(std::string_view iso_week,
                                           ReportFormat format) -> std::string {
  return query_service_->RunWeeklyQuery(iso_week, format);
}

auto ReportGenerator::GenerateYearlyReport(std::string_view year,
                                           ReportFormat format) -> std::string {
  return query_service_->RunYearlyQuery(year, format);
}

auto ReportGenerator::GenerateAllDailyReports(ReportFormat format)
    -> FormattedGroupedReports {
  return query_service_->RunExportAllDailyReportsQuery(format);
}

auto ReportGenerator::GenerateAllMonthlyReports(ReportFormat format)
    -> FormattedMonthlyReports {
  return query_service_->RunExportAllMonthlyReportsQuery(format);
}

auto ReportGenerator::GenerateAllPeriodReports(
    const std::vector<int>& days_list, ReportFormat format)
    -> FormattedPeriodReports {
  return query_service_->RunExportAllPeriodReportsQuery(days_list, format);
}

auto ReportGenerator::GenerateAllWeeklyReports(ReportFormat format)
    -> FormattedWeeklyReports {
  return query_service_->RunExportAllWeeklyReportsQuery(format);
}

auto ReportGenerator::GenerateAllYearlyReports(ReportFormat format)
    -> FormattedYearlyReports {
  return query_service_->RunExportAllYearlyReportsQuery(format);
}
