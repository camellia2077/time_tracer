// infra/reporting/exporter.hpp
#ifndef INFRASTRUCTURE_REPORTS_EXPORTER_H_
#define INFRASTRUCTURE_REPORTS_EXPORTER_H_

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "application/compat/reporting/i_report_exporter.hpp"
#include "infra/reporting/report_file_manager.hpp"

namespace tracer::core::infrastructure::reports {
class Exporter : public IReportExporter {
 public:
  explicit Exporter(const std::filesystem::path& export_root_path);
  ~Exporter() override;

  auto ExportSingleDayReport(const SingleExportTask& task,
                             ReportFormat format) const -> void override;
  auto ExportSingleMonthReport(const SingleExportTask& task,
                               ReportFormat format) const -> void override;
  auto ExportSinglePeriodReport(int days, std::string_view content,
                                ReportFormat format) const -> void override;
  auto ExportSingleWeekReport(const SingleExportTask& task,
                              ReportFormat format) const -> void override;
  auto ExportSingleYearReport(const SingleExportTask& task,
                              ReportFormat format) const -> void override;

  auto ExportAllDailyReports(const FormattedGroupedReports& reports,
                             ReportFormat format) const -> void override;
  auto ExportAllMonthlyReports(const FormattedMonthlyReports& reports,
                               ReportFormat format) const -> void override;
  auto ExportAllPeriodReports(const FormattedPeriodReports& reports,
                              ReportFormat format) const -> void override;
  auto ExportAllWeeklyReports(const FormattedWeeklyReports& reports,
                              ReportFormat format) const -> void override;
  auto ExportAllYearlyReports(const FormattedYearlyReports& reports,
                              ReportFormat format) const -> void override;

 private:
  static void WriteReportToFile(std::string_view report_content,
                                const std::filesystem::path& output_path);
  std::unique_ptr<ReportFileManager> file_manager_;
};

}  // namespace tracer::core::infrastructure::reports

using Exporter = tracer::core::infrastructure::reports::Exporter;

#endif  // INFRASTRUCTURE_REPORTS_EXPORTER_H_
