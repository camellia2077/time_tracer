// application/reporting/generator/report_file_manager.hpp
#ifndef APPLICATION_REPORTING_GENERATOR_REPORT_FILE_MANAGER_H_
#define APPLICATION_REPORTING_GENERATOR_REPORT_FILE_MANAGER_H_

#include <filesystem>
#include <string>
#include <vector>

#include "reports/shared/types/report_format.hpp"

namespace fs = std::filesystem;

class ReportFileManager {
 public:
  explicit ReportFileManager(fs::path export_root);

  fs::path get_single_day_report_path(const std::string& date,
                                      ReportFormat format) const;
  fs::path get_single_month_report_path(const std::string& month,
                                        ReportFormat format) const;
  fs::path get_single_period_report_path(int days, ReportFormat format) const;
  fs::path get_single_week_report_path(const std::string& iso_week,
                                       ReportFormat format) const;
  fs::path get_single_year_report_path(const std::string& year,
                                       ReportFormat format) const;

  fs::path get_all_daily_reports_base_dir(ReportFormat format) const;
  fs::path get_all_monthly_reports_base_dir(ReportFormat format) const;
  fs::path get_all_period_reports_base_dir(ReportFormat format) const;
  fs::path get_all_weekly_reports_base_dir(ReportFormat format) const;
  fs::path get_all_yearly_reports_base_dir(ReportFormat format) const;

 private:
  fs::path export_root_path_;
};

#endif  // APPLICATION_REPORTING_GENERATOR_REPORT_FILE_MANAGER_H_
