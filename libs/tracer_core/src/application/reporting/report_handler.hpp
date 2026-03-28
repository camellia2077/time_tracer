// application/reporting/report_handler.hpp
#ifndef APPLICATION_REPORTING_REPORT_HANDLER_H_
#define APPLICATION_REPORTING_REPORT_HANDLER_H_

#include <memory>

#include "application/compat/reporting/i_report_handler.hpp"

class IReportQueryService;

class ReportHandler : public IReportHandler {
 public:
  explicit ReportHandler(std::unique_ptr<IReportQueryService> query_service);
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

 private:
  std::unique_ptr<IReportQueryService> query_service_;
};

#endif  // APPLICATION_REPORTING_REPORT_HANDLER_H_
