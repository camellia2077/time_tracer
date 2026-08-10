module;

#include "infra/sqlite_fwd.hpp"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "application/ports/insights/i_platform_clock.hpp"
#include "application/ports/insights/i_insights_data_query_service.hpp"
#include "infra/config/models/insights_config_models.hpp"

export module tracer.core.infrastructure.insights.data_querying
    .sqlite_insights_data_query_service;

export namespace tracer::core::infrastructure::insights {
class SqliteInsightsDataQueryService final
    : public tracer_core::application::ports::IInsightsDataQueryService {
 public:
  SqliteInsightsDataQueryService(
      sqlite3* db_connection,
      std::shared_ptr<tracer_core::application::ports::IPlatformClock>
          platform_clock,
      DailyStatusConfig status_config = {});

  auto QueryDaily(std::string_view date) -> DailyInsightsData override;
  auto QueryMonthly(std::string_view month) -> MonthlyInsightsData override;
  auto QueryPeriod(int days) -> PeriodInsightsData override;
  auto QueryRange(std::string_view start_date, std::string_view end_date)
      -> PeriodInsightsData override;
  auto QueryWeekly(std::string_view iso_week) -> WeeklyInsightsData override;
  auto QueryYearly(std::string_view year) -> YearlyInsightsData override;

  auto ListDailyTargets() -> std::vector<std::string> override;
  auto ListMonthlyTargets() -> std::vector<std::string> override;
  auto ListWeeklyTargets() -> std::vector<std::string> override;
  auto ListYearlyTargets() -> std::vector<std::string> override;

  auto QueryPeriodBatch(const std::vector<int>& days_list)
      -> std::map<int, PeriodInsightsData> override;
  auto QueryAllDaily() -> std::map<std::string, DailyInsightsData> override;
  auto QueryAllMonthly() -> std::map<std::string, MonthlyInsightsData> override;
  auto QueryAllWeekly() -> std::map<std::string, WeeklyInsightsData> override;
  auto QueryAllYearly() -> std::map<std::string, YearlyInsightsData> override;

 private:
  sqlite3* db_connection_;
  std::shared_ptr<tracer_core::application::ports::IPlatformClock>
      platform_clock_;
  DailyStatusConfig status_config_;
};

}  // namespace tracer::core::infrastructure::insights
