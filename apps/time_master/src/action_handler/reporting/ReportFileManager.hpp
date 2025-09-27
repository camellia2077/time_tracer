// action_handler/reporting/ReportFileManager.hpp
#ifndef REPORT_FILE_MANAGER_HPP
#define REPORT_FILE_MANAGER_HPP

#include <filesystem>
#include <string>
#include <vector>
#include "reports/shared/types/ReportFormat.hpp"

namespace fs = std::filesystem;

class ReportFileManager {
public:
    explicit ReportFileManager(const fs::path& export_root);

    fs::path get_single_day_report_path(const std::string& date, ReportFormat format) const;
    fs::path get_single_month_report_path(const std::string& month, ReportFormat format) const;
    fs::path get_single_period_report_path(int days, ReportFormat format) const;

    fs::path get_all_daily_reports_base_dir(ReportFormat format) const;
    fs::path get_all_monthly_reports_base_dir(ReportFormat format) const;
    fs::path get_all_period_reports_base_dir(ReportFormat format) const;

private:
    fs::path export_root_path_;
};

#endif // REPORT_FILE_MANAGER_HPP