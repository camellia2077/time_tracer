// infrastructure/reports/report_file_manager.hpp
#ifndef INFRASTRUCTURE_REPORTS_REPORT_FILE_MANAGER_H_
#define INFRASTRUCTURE_REPORTS_REPORT_FILE_MANAGER_H_

#include <filesystem>
#include <string>
#include <vector>

#include "reports/shared/types/report_format.hpp"

namespace fs = std::filesystem;

class ReportFileManager {
 public:
  explicit ReportFileManager(fs::path export_root);

  auto GetSingleDayReportPath(const std::string& date,
                              ReportFormat format) const -> fs::path;
  auto GetSingleMonthReportPath(const std::string& month,
                                ReportFormat format) const -> fs::path;
  auto GetSinglePeriodReportPath(int days, ReportFormat format) const
      -> fs::path;
  auto GetSingleWeekReportPath(const std::string& iso_week,
                               ReportFormat format) const -> fs::path;
  auto GetSingleYearReportPath(const std::string& year,
                               ReportFormat format) const -> fs::path;

  auto GetAllDailyReportsBaseDir(ReportFormat format) const -> fs::path;
  auto GetAllMonthlyReportsBaseDir(ReportFormat format) const -> fs::path;
  auto GetAllPeriodReportsBaseDir(ReportFormat format) const -> fs::path;
  auto GetAllWeeklyReportsBaseDir(ReportFormat format) const -> fs::path;
  auto GetAllYearlyReportsBaseDir(ReportFormat format) const -> fs::path;

 private:
  fs::path export_root_path_;
};

#endif  // INFRASTRUCTURE_REPORTS_REPORT_FILE_MANAGER_H_
