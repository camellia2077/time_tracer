// application/reporting/generator/report_file_manager.cpp
#include "application/reporting/generator/report_file_manager.hpp"

#include <stdexcept>
#include <utility>

#include "infrastructure/reports/export_utils.hpp"

ReportFileManager::ReportFileManager(fs::path export_root)
    : export_root_path_(std::move(export_root)) {}

auto ReportFileManager::get_single_day_report_path(const std::string& date,
                                                   ReportFormat format) const
    -> fs::path {
  auto details = ExportUtils::get_report_format_details(format).value();
  return export_root_path_ / details.dir_name / "day" /
         (date + details.extension);
}

auto ReportFileManager::get_single_month_report_path(const std::string& month,
                                                     ReportFormat format) const
    -> fs::path {
  auto details = ExportUtils::get_report_format_details(format).value();
  return export_root_path_ / details.dir_name / "month" /
         (month + details.extension);
}

auto ReportFileManager::get_single_period_report_path(int days,
                                                      ReportFormat format) const
    -> fs::path {
  auto details = ExportUtils::get_report_format_details(format).value();
  return export_root_path_ / details.dir_name / "recent" /
         ("last_" + std::to_string(days) + "_days_report" + details.extension);
}

auto ReportFileManager::get_single_week_report_path(const std::string& iso_week,
                                                    ReportFormat format) const
    -> fs::path {
  auto details = ExportUtils::get_report_format_details(format).value();
  return export_root_path_ / details.dir_name / "week" /
         (iso_week + details.extension);
}

auto ReportFileManager::get_single_year_report_path(const std::string& year,
                                                    ReportFormat format) const
    -> fs::path {
  auto details = ExportUtils::get_report_format_details(format).value();
  return export_root_path_ / details.dir_name / "year" /
         (year + details.extension);
}

auto ReportFileManager::get_all_daily_reports_base_dir(
    ReportFormat format) const -> fs::path {
  auto details = ExportUtils::get_report_format_details(format).value();
  return export_root_path_ / details.dir_name / "day";
}

auto ReportFileManager::get_all_monthly_reports_base_dir(
    ReportFormat format) const -> fs::path {
  auto details = ExportUtils::get_report_format_details(format).value();
  return export_root_path_ / details.dir_name / "month";
}

auto ReportFileManager::get_all_period_reports_base_dir(
    ReportFormat format) const -> fs::path {
  auto details = ExportUtils::get_report_format_details(format).value();
  return export_root_path_ / details.dir_name / "recent";
}

auto ReportFileManager::get_all_weekly_reports_base_dir(
    ReportFormat format) const -> fs::path {
  auto details = ExportUtils::get_report_format_details(format).value();
  return export_root_path_ / details.dir_name / "week";
}

auto ReportFileManager::get_all_yearly_reports_base_dir(
    ReportFormat format) const -> fs::path {
  auto details = ExportUtils::get_report_format_details(format).value();
  return export_root_path_ / details.dir_name / "year";
}
