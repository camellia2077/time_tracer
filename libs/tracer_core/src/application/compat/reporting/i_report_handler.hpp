// application/compat/reporting/i_report_handler.hpp
#ifndef APPLICATION_COMPAT_REPORTING_I_REPORT_HANDLER_H_
#define APPLICATION_COMPAT_REPORTING_I_REPORT_HANDLER_H_

#include <string>
#include <string_view>
#include <vector>

#include "domain/reports/types/report_types.hpp"

class IReportHandler {
 public:
  virtual ~IReportHandler() = default;

  virtual auto RunDailyQuery(std::string_view date, ReportFormat format)
      -> std::string = 0;
  virtual auto RunMonthlyQuery(std::string_view month, ReportFormat format)
      -> std::string = 0;
  virtual auto RunPeriodQuery(int days, ReportFormat format) -> std::string = 0;
  virtual auto RunWeeklyQuery(std::string_view iso_week, ReportFormat format)
      -> std::string = 0;
  virtual auto RunYearlyQuery(std::string_view year, ReportFormat format)
      -> std::string = 0;
  virtual auto RunPeriodQueries(const std::vector<int>& days_list,
                                ReportFormat format) -> std::string = 0;

  virtual auto RunExportSingleDayReport(std::string_view date,
                                        ReportFormat format) -> void = 0;
  virtual auto RunExportSingleMonthReport(std::string_view month,
                                          ReportFormat format) -> void = 0;
  virtual auto RunExportSinglePeriodReport(int days, ReportFormat format)
      -> void = 0;
  virtual auto RunExportSingleWeekReport(std::string_view iso_week,
                                         ReportFormat format) -> void = 0;
  virtual auto RunExportSingleYearReport(std::string_view year,
                                         ReportFormat format) -> void = 0;
  virtual auto RunExportAllDailyReportsQuery(ReportFormat format) -> void = 0;
  virtual auto RunExportAllMonthlyReportsQuery(ReportFormat format) -> void = 0;
  virtual auto RunExportAllPeriodReportsQuery(const std::vector<int>& days_list,
                                              ReportFormat format) -> void = 0;
  virtual auto RunExportAllWeeklyReportsQuery(ReportFormat format) -> void = 0;
  virtual auto RunExportAllYearlyReportsQuery(ReportFormat format) -> void = 0;
};

#endif  // APPLICATION_COMPAT_REPORTING_I_REPORT_HANDLER_H_
