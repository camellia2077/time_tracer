// action_handler/reporting/ReportFileManager.cpp
#include "ReportFileManager.hpp"
#include "ExportUtils.hpp"
#include <stdexcept>

ReportFileManager::ReportFileManager(const fs::path& export_root)
    : export_root_path_(export_root) {}

fs::path ReportFileManager::get_single_day_report_path(const std::string& date, ReportFormat format) const {
    auto details = ExportUtils::get_report_format_details(format).value();
    return export_root_path_ / details.dir_name / "daily" / (date + details.extension);
}

fs::path ReportFileManager::get_single_month_report_path(const std::string& month, ReportFormat format) const {
    auto details = ExportUtils::get_report_format_details(format).value();
    return export_root_path_ / details.dir_name / "monthly" / (month + details.extension);
}

fs::path ReportFileManager::get_single_period_report_path(int days, ReportFormat format) const {
    auto details = ExportUtils::get_report_format_details(format).value();
    return export_root_path_ / details.dir_name / "periods" / ("Last_" + std::to_string(days) + "_Days_Report" + details.extension);
}

fs::path ReportFileManager::get_all_daily_reports_base_dir(ReportFormat format) const {
    auto details = ExportUtils::get_report_format_details(format).value();
    return export_root_path_ / details.dir_name / "days";
}

fs::path ReportFileManager::get_all_monthly_reports_base_dir(ReportFormat format) const {
    auto details = ExportUtils::get_report_format_details(format).value();
    return export_root_path_ / details.dir_name / "monthly";
}

fs::path ReportFileManager::get_all_period_reports_base_dir(ReportFormat format) const {
    auto details = ExportUtils::get_report_format_details(format).value();
    return export_root_path_ / details.dir_name / "periods";
}