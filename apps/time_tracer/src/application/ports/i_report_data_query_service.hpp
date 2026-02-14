// application/ports/i_report_data_query_service.hpp
#ifndef APPLICATION_PORTS_I_REPORT_DATA_QUERY_SERVICE_H_
#define APPLICATION_PORTS_I_REPORT_DATA_QUERY_SERVICE_H_

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/period_report_models.hpp"

namespace time_tracer::application::ports {

class IReportDataQueryService {
 public:
  virtual ~IReportDataQueryService() = default;

  virtual auto QueryDaily(std::string_view date) -> DailyReportData = 0;
  virtual auto QueryMonthly(std::string_view month) -> MonthlyReportData = 0;
  virtual auto QueryPeriod(int days) -> PeriodReportData = 0;
  virtual auto QueryWeekly(std::string_view iso_week) -> WeeklyReportData = 0;
  virtual auto QueryYearly(std::string_view year) -> YearlyReportData = 0;

  virtual auto QueryPeriodBatch(const std::vector<int>& days_list)
      -> std::map<int, PeriodReportData> = 0;
  virtual auto QueryAllDaily() -> std::map<std::string, DailyReportData> = 0;
  virtual auto QueryAllMonthly()
      -> std::map<std::string, MonthlyReportData> = 0;
  virtual auto QueryAllWeekly() -> std::map<std::string, WeeklyReportData> = 0;
  virtual auto QueryAllYearly() -> std::map<std::string, YearlyReportData> = 0;
};

}  // namespace time_tracer::application::ports

#endif  // APPLICATION_PORTS_I_REPORT_DATA_QUERY_SERVICE_H_
