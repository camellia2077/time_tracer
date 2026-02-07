// infrastructure/reports/report_file_manager.cpp
#include "infrastructure/reports/report_file_manager.hpp"

#include <stdexcept>
#include <utility>

#include "infrastructure/reports/export_utils.hpp"

ReportFileManager::ReportFileManager(fs::path export_root)
    : export_root_path_(std::move(export_root)) {}

auto ReportFileManager::GetSingleDayReportPath(const std::string& date,
                                               ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "day" /
         (date + details.extension);
}

auto ReportFileManager::GetSingleMonthReportPath(const std::string& month,
                                                 ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "month" /
         (month + details.extension);
}

auto ReportFileManager::GetSinglePeriodReportPath(int days,
                                                  ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "recent" /
         ("last_" + std::to_string(days) + "_days_report" + details.extension);
}

auto ReportFileManager::GetSingleWeekReportPath(const std::string& iso_week,
                                                ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "week" /
         (iso_week + details.extension);
}

auto ReportFileManager::GetSingleYearReportPath(const std::string& year,
                                                ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "year" /
         (year + details.extension);
}

auto ReportFileManager::GetAllDailyReportsBaseDir(ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "day";
}

auto ReportFileManager::GetAllMonthlyReportsBaseDir(ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "month";
}

auto ReportFileManager::GetAllPeriodReportsBaseDir(ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "recent";
}

auto ReportFileManager::GetAllWeeklyReportsBaseDir(ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "week";
}

auto ReportFileManager::GetAllYearlyReportsBaseDir(ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "year";
}
