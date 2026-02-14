// infrastructure/reports/sqlite_report_data_query_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SQLITE_REPORT_DATA_QUERY_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SQLITE_REPORT_DATA_QUERY_SERVICE_H_

#include <sqlite3.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "application/ports/i_platform_clock.hpp"
#include "application/ports/i_report_data_query_service.hpp"

namespace infrastructure::reports {

class SqliteReportDataQueryService final
    : public time_tracer::application::ports::IReportDataQueryService {
 public:
  SqliteReportDataQueryService(
      sqlite3* db_connection,
      std::shared_ptr<time_tracer::application::ports::IPlatformClock>
          platform_clock);

  auto QueryDaily(std::string_view date) -> DailyReportData override;
  auto QueryMonthly(std::string_view month) -> MonthlyReportData override;
  auto QueryPeriod(int days) -> PeriodReportData override;
  auto QueryWeekly(std::string_view iso_week) -> WeeklyReportData override;
  auto QueryYearly(std::string_view year) -> YearlyReportData override;

  auto QueryPeriodBatch(const std::vector<int>& days_list)
      -> std::map<int, PeriodReportData> override;
  auto QueryAllDaily() -> std::map<std::string, DailyReportData> override;
  auto QueryAllMonthly() -> std::map<std::string, MonthlyReportData> override;
  auto QueryAllWeekly() -> std::map<std::string, WeeklyReportData> override;
  auto QueryAllYearly() -> std::map<std::string, YearlyReportData> override;

 private:
  sqlite3* db_connection_;
  std::shared_ptr<time_tracer::application::ports::IPlatformClock>
      platform_clock_;
};

}  // namespace infrastructure::reports

#endif  // INFRASTRUCTURE_REPORTS_SQLITE_REPORT_DATA_QUERY_SERVICE_H_
