// application/reporting/report_handler.cpp
#include "application/reporting/report_handler.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include "application/reporting/generator/report_generator.hpp"
#include "infrastructure/reports/exporter.hpp"

namespace {
constexpr int kSeparatorLength = 40;
}

ReportHandler::ReportHandler(std::unique_ptr<ReportGenerator> generator,
                             std::unique_ptr<Exporter> exporter)
    : generator_(std::move(generator)), exporter_(std::move(exporter)) {}

ReportHandler::~ReportHandler() = default;

auto ReportHandler::RunDailyQuery(std::string_view date, ReportFormat format)
    -> std::string {
  return generator_->GenerateDailyReport(date, format);
}

auto ReportHandler::RunMonthlyQuery(std::string_view month, ReportFormat format)
    -> std::string {
  return generator_->GenerateMonthlyReport(month, format);
}

auto ReportHandler::RunPeriodQuery(int days, ReportFormat format)
    -> std::string {
  return generator_->GeneratePeriodReport(days, format);
}

auto ReportHandler::RunWeeklyQuery(std::string_view iso_week,
                                   ReportFormat format) -> std::string {
  return generator_->GenerateWeeklyReport(iso_week, format);
}

auto ReportHandler::RunYearlyQuery(std::string_view year, ReportFormat format)
    -> std::string {
  return generator_->GenerateYearlyReport(year, format);
}

// [新增] 批量查询逻辑下沉至 Core
auto ReportHandler::RunPeriodQueries(const std::vector<int>& days_list,
                                     ReportFormat format) -> std::string {
  std::ostringstream oss;
  for (size_t i = 0; i < days_list.size(); ++i) {
    if (i > 0) {
      oss << "\n" << std::string(kSeparatorLength, '-') << "\n";
    }
    try {
      oss << RunPeriodQuery(days_list[i], format);
    } catch (const std::exception& e) {
      oss << "Error querying period " << days_list[i] << " days: " << e.what();
    }
  }
  return oss.str();
}

auto ReportHandler::RunExportSingleDayReport(std::string_view date,
                                             ReportFormat format) -> void {
  auto content = generator_->GenerateDailyReport(date, format);
  // [修复] 调用 ExportSingleDayReport (之前错误调用了
  // export_all_daily_reports)
  exporter_->ExportSingleDayReport({.id = date, .content = content}, format);
}

auto ReportHandler::RunExportSingleMonthReport(std::string_view month,
                                               ReportFormat format) -> void {
  auto content = generator_->GenerateMonthlyReport(month, format);
  // [修复] 调用 ExportSingleMonthReport
  exporter_->ExportSingleMonthReport({.id = month, .content = content}, format);
}

auto ReportHandler::RunExportSinglePeriodReport(int days, ReportFormat format)
    -> void {
  auto content = generator_->GeneratePeriodReport(days, format);
  // [修复] 调用 ExportSinglePeriodReport (之前拼写错误或不存在
  // 这个函数)
  exporter_->ExportSinglePeriodReport(days, content, format);
}

auto ReportHandler::RunExportSingleWeekReport(std::string_view iso_week,
                                              ReportFormat format) -> void {
  auto content = generator_->GenerateWeeklyReport(iso_week, format);
  exporter_->ExportSingleWeekReport({.id = iso_week, .content = content},
                                    format);
}

auto ReportHandler::RunExportSingleYearReport(std::string_view year,
                                              ReportFormat format) -> void {
  auto content = generator_->GenerateYearlyReport(year, format);
  exporter_->ExportSingleYearReport({.id = year, .content = content}, format);
}

auto ReportHandler::RunExportAllDailyReportsQuery(ReportFormat format) -> void {
  // [修复] 获取整个集合
  auto reports = generator_->GenerateAllDailyReports(format);
  // [修复] 直接传递集合给 Exporter，不要手动遍历，Exporter
  // 内部会处理遍历和目录结构
  exporter_->ExportAllDailyReports(reports, format);
}

auto ReportHandler::RunExportAllMonthlyReportsQuery(ReportFormat format)
    -> void {
  // [修复] 获取整个集合
  auto reports = generator_->GenerateAllMonthlyReports(format);
  // [修复] 直接传递集合
  exporter_->ExportAllMonthlyReports(reports, format);
}

auto ReportHandler::RunExportAllPeriodReportsQuery(
    const std::vector<int>& days_list, ReportFormat format) -> void {
  // 针对 Period，我们也可以优化为使用 Generator 和 Exporter 的批量接口
  // 这样可以利用 Exporter 中的 ExportUtils::execute_export_task
  // 提供的统一日志 and 目录管理
  auto reports = generator_->GenerateAllPeriodReports(days_list, format);
  exporter_->ExportAllPeriodReports(reports, format);
}

auto ReportHandler::RunExportAllWeeklyReportsQuery(ReportFormat format) -> void {
  auto reports = generator_->GenerateAllWeeklyReports(format);
  exporter_->ExportAllWeeklyReports(reports, format);
}

auto ReportHandler::RunExportAllYearlyReportsQuery(ReportFormat format) -> void {
  auto reports = generator_->GenerateAllYearlyReports(format);
  exporter_->ExportAllYearlyReports(reports, format);
}
