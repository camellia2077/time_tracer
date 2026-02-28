// application/reporting/report_handler.hpp
#ifndef APPLICATION_REPORTING_REPORT_HANDLER_H_
#define APPLICATION_REPORTING_REPORT_HANDLER_H_

#include <memory>

#include "application/interfaces/i_report_handler.hpp"

class IReportExporter;
class IReportQueryService;

class ReportHandler : public IReportHandler {
 public:
  ReportHandler(std::unique_ptr<IReportQueryService> query_service,
                std::unique_ptr<IReportExporter> exporter);
  ~ReportHandler() override;

  auto RunDailyQuery(std::string_view date, ReportFormat format)
      -> std::string override;
  auto RunMonthlyQuery(std::string_view month, ReportFormat format)
      -> std::string override;
  auto RunPeriodQuery(int days, ReportFormat format) -> std::string override;
  auto RunWeeklyQuery(std::string_view iso_week, ReportFormat format)
      -> std::string override;
  auto RunYearlyQuery(std::string_view year, ReportFormat format)
      -> std::string override;
  auto RunPeriodQueries(const std::vector<int>& days_list, ReportFormat format)
      -> std::string override;

  auto RunExportSingleDayReport(std::string_view date, ReportFormat format)
      -> void override;
  auto RunExportSingleMonthReport(std::string_view month, ReportFormat format)
      -> void override;
  auto RunExportSinglePeriodReport(int days, ReportFormat format)
      -> void override;
  auto RunExportSingleWeekReport(std::string_view iso_week, ReportFormat format)
      -> void override;
  auto RunExportSingleYearReport(std::string_view year, ReportFormat format)
      -> void override;
  auto RunExportAllDailyReportsQuery(ReportFormat format) -> void override;
  auto RunExportAllMonthlyReportsQuery(ReportFormat format) -> void override;
  auto RunExportAllPeriodReportsQuery(const std::vector<int>& days_list,
                                      ReportFormat format) -> void override;
  auto RunExportAllWeeklyReportsQuery(ReportFormat format) -> void override;
  auto RunExportAllYearlyReportsQuery(ReportFormat format) -> void override;

 private:
  std::unique_ptr<IReportQueryService> query_service_;
  std::unique_ptr<IReportExporter> exporter_;
};

#endif  // APPLICATION_REPORTING_REPORT_HANDLER_H_
