// infrastructure/reports/report_file_manager.hpp
#ifndef INFRASTRUCTURE_REPORTS_REPORT_FILE_MANAGER_H_
#define INFRASTRUCTURE_REPORTS_REPORT_FILE_MANAGER_H_

#include <filesystem>
#include <string>
#include <vector>

#include "domain/reports/types/report_format.hpp"

namespace fs = std::filesystem;

class ReportFileManager {
 public:
  explicit ReportFileManager(fs::path export_root);

  [[nodiscard]] auto GetSingleDayReportPath(const std::string& date,
                                            ReportFormat format) const
      -> fs::path;
  [[nodiscard]] auto GetSingleMonthReportPath(const std::string& month,
                                              ReportFormat format) const
      -> fs::path;
  [[nodiscard]] auto GetSinglePeriodReportPath(int days,
                                               ReportFormat format) const
      -> fs::path;
  [[nodiscard]] auto GetSingleWeekReportPath(const std::string& iso_week,
                                             ReportFormat format) const
      -> fs::path;
  [[nodiscard]] auto GetSingleYearReportPath(const std::string& year,
                                             ReportFormat format) const
      -> fs::path;

  [[nodiscard]] auto GetAllDailyReportsBaseDir(ReportFormat format) const
      -> fs::path;
  [[nodiscard]] auto GetAllMonthlyReportsBaseDir(ReportFormat format) const
      -> fs::path;
  [[nodiscard]] auto GetAllPeriodReportsBaseDir(ReportFormat format) const
      -> fs::path;
  [[nodiscard]] auto GetAllWeeklyReportsBaseDir(ReportFormat format) const
      -> fs::path;
  [[nodiscard]] auto GetAllYearlyReportsBaseDir(ReportFormat format) const
      -> fs::path;

 private:
  fs::path export_root_path_;
};

#endif  // INFRASTRUCTURE_REPORTS_REPORT_FILE_MANAGER_H_
