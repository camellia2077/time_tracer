// application/reporting/report_handler.cpp
#include "application/reporting/report_handler.hpp"

#include <sstream>
#include <string>
#include <utility>

#include "application/interfaces/i_report_exporter.hpp"
#include "application/interfaces/i_report_query_service.hpp"

namespace {
constexpr int kSeparatorLength = 40;
}

ReportHandler::ReportHandler(std::unique_ptr<IReportQueryService> query_service,
                             std::unique_ptr<IReportExporter> exporter)
    : query_service_(std::move(query_service)),
      exporter_(std::move(exporter)) {}

ReportHandler::~ReportHandler() = default;

auto ReportHandler::RunDailyQuery(std::string_view date, ReportFormat format)
    -> std::string {
  return query_service_->RunDailyQuery(date, format);
}

auto ReportHandler::RunMonthlyQuery(std::string_view month, ReportFormat format)
    -> std::string {
  return query_service_->RunMonthlyQuery(month, format);
}

auto ReportHandler::RunPeriodQuery(int days, ReportFormat format)
    -> std::string {
  return query_service_->RunPeriodQuery(days, format);
}

auto ReportHandler::RunWeeklyQuery(std::string_view iso_week,
                                   ReportFormat format) -> std::string {
  return query_service_->RunWeeklyQuery(iso_week, format);
}

auto ReportHandler::RunYearlyQuery(std::string_view year, ReportFormat format)
    -> std::string {
  return query_service_->RunYearlyQuery(year, format);
}

auto ReportHandler::RunPeriodQueries(const std::vector<int>& days_list,
                                     ReportFormat format) -> std::string {
  std::ostringstream output;
  for (size_t index = 0; index < days_list.size(); ++index) {
    if (index > 0) {
      output << "\n" << std::string(kSeparatorLength, '-') << "\n";
    }
    try {
      output << RunPeriodQuery(days_list[index], format);
    } catch (const std::exception& exception) {
      output << "Error querying period " << days_list[index]
             << " days: " << exception.what();
    }
  }
  return output.str();
}

auto ReportHandler::RunExportSingleDayReport(std::string_view date,
                                             ReportFormat format) -> void {
  const auto kContent = query_service_->RunDailyQuery(date, format);
  exporter_->ExportSingleDayReport({.id = date, .kContent = kContent}, format);
}

auto ReportHandler::RunExportSingleMonthReport(std::string_view month,
                                               ReportFormat format) -> void {
  const auto kContent = query_service_->RunMonthlyQuery(month, format);
  exporter_->ExportSingleMonthReport({.id = month, .kContent = kContent},
                                     format);
}

auto ReportHandler::RunExportSinglePeriodReport(int days, ReportFormat format)
    -> void {
  const auto kContent = query_service_->RunPeriodQuery(days, format);
  exporter_->ExportSinglePeriodReport(days, kContent, format);
}

auto ReportHandler::RunExportSingleWeekReport(std::string_view iso_week,
                                              ReportFormat format) -> void {
  const auto kContent = query_service_->RunWeeklyQuery(iso_week, format);
  exporter_->ExportSingleWeekReport({.id = iso_week, .kContent = kContent},
                                    format);
}

auto ReportHandler::RunExportSingleYearReport(std::string_view year,
                                              ReportFormat format) -> void {
  const auto kContent = query_service_->RunYearlyQuery(year, format);
  exporter_->ExportSingleYearReport({.id = year, .kContent = kContent}, format);
}

auto ReportHandler::RunExportAllDailyReportsQuery(ReportFormat format) -> void {
  const auto kReports = query_service_->RunExportAllDailyReportsQuery(format);
  exporter_->ExportAllDailyReports(kReports, format);
}

auto ReportHandler::RunExportAllMonthlyReportsQuery(ReportFormat format)
    -> void {
  const auto kReports = query_service_->RunExportAllMonthlyReportsQuery(format);
  exporter_->ExportAllMonthlyReports(kReports, format);
}

auto ReportHandler::RunExportAllPeriodReportsQuery(
    const std::vector<int>& days_list, ReportFormat format) -> void {
  const auto kReports =
      query_service_->RunExportAllPeriodReportsQuery(days_list, format);
  exporter_->ExportAllPeriodReports(kReports, format);
}

auto ReportHandler::RunExportAllWeeklyReportsQuery(ReportFormat format)
    -> void {
  const auto kReports = query_service_->RunExportAllWeeklyReportsQuery(format);
  exporter_->ExportAllWeeklyReports(kReports, format);
}

auto ReportHandler::RunExportAllYearlyReportsQuery(ReportFormat format)
    -> void {
  const auto kReports = query_service_->RunExportAllYearlyReportsQuery(format);
  exporter_->ExportAllYearlyReports(kReports, format);
}
