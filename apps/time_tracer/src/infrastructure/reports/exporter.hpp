// infrastructure/reports/exporter.hpp
#ifndef INFRASTRUCTURE_REPORTS_EXPORTER_H_
#define INFRASTRUCTURE_REPORTS_EXPORTER_H_

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "reports/data/model/query_data_structs.hpp"
#include "reports/shared/types/report_format.hpp"

namespace fs = std::filesystem;
class ReportFileManager;

struct SingleExportTask {
  std::string_view id;
  std::string_view content;
};

class Exporter {
 public:
  explicit Exporter(const fs::path& export_root_path);
  ~Exporter();

  void ExportSingleDayReport(const SingleExportTask& task,
                             ReportFormat format) const;
  void ExportSingleMonthReport(const SingleExportTask& task,
                               ReportFormat format) const;
  void ExportSinglePeriodReport(int days, std::string_view content,
                                ReportFormat format) const;
  void ExportSingleWeekReport(const SingleExportTask& task,
                              ReportFormat format) const;
  void ExportSingleYearReport(const SingleExportTask& task,
                              ReportFormat format) const;

  void ExportAllDailyReports(const FormattedGroupedReports& reports,
                             ReportFormat format) const;
  void ExportAllMonthlyReports(const FormattedMonthlyReports& reports,
                               ReportFormat format) const;
  void ExportAllPeriodReports(const FormattedPeriodReports& reports,
                              ReportFormat format) const;
  void ExportAllWeeklyReports(const FormattedWeeklyReports& reports,
                              ReportFormat format) const;
  void ExportAllYearlyReports(const FormattedYearlyReports& reports,
                              ReportFormat format) const;

 private:
  static void WriteReportToFile(std::string_view report_content,
                                const fs::path& output_path);
  std::unique_ptr<ReportFileManager> file_manager_;
};
#endif  // INFRASTRUCTURE_REPORTS_EXPORTER_H_
