// application/interfaces/i_report_exporter.hpp
#ifndef APPLICATION_INTERFACES_I_REPORT_EXPORTER_H_
#define APPLICATION_INTERFACES_I_REPORT_EXPORTER_H_

#include <string_view>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"

struct SingleExportTask {
  std::string_view id;
  std::string_view kContent;
};

class IReportExporter {
 public:
  virtual ~IReportExporter() = default;

  virtual auto ExportSingleDayReport(const SingleExportTask& task,
                                     ReportFormat format) const -> void = 0;
  virtual auto ExportSingleMonthReport(const SingleExportTask& task,
                                       ReportFormat format) const -> void = 0;
  virtual auto ExportSinglePeriodReport(int days, std::string_view content,
                                        ReportFormat format) const -> void = 0;
  virtual auto ExportSingleWeekReport(const SingleExportTask& task,
                                      ReportFormat format) const -> void = 0;
  virtual auto ExportSingleYearReport(const SingleExportTask& task,
                                      ReportFormat format) const -> void = 0;

  virtual auto ExportAllDailyReports(const FormattedGroupedReports& reports,
                                     ReportFormat format) const -> void = 0;
  virtual auto ExportAllMonthlyReports(const FormattedMonthlyReports& reports,
                                       ReportFormat format) const -> void = 0;
  virtual auto ExportAllPeriodReports(const FormattedPeriodReports& reports,
                                      ReportFormat format) const -> void = 0;
  virtual auto ExportAllWeeklyReports(const FormattedWeeklyReports& reports,
                                      ReportFormat format) const -> void = 0;
  virtual auto ExportAllYearlyReports(const FormattedYearlyReports& reports,
                                      ReportFormat format) const -> void = 0;
};

#endif  // APPLICATION_INTERFACES_I_REPORT_EXPORTER_H_
