// application/ports/insights/i_insights_data_query_service.hpp
#ifndef APPLICATION_PORTS_I_INSIGHTS_DATA_QUERY_SERVICE_H_
#define APPLICATION_PORTS_I_INSIGHTS_DATA_QUERY_SERVICE_H_

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "domain/insights/models/daily_insights_data.hpp"
#include "domain/insights/models/period_insights_models.hpp"

namespace tracer_core::application::ports {

class IInsightsDataQueryService {
 public:
  virtual ~IInsightsDataQueryService() = default;

  virtual auto QueryDaily(std::string_view date) -> DailyInsightsData = 0;
  virtual auto QueryMonthly(std::string_view month) -> MonthlyInsightsData = 0;
  virtual auto QueryPeriod(int days) -> PeriodInsightsData = 0;
  virtual auto QueryRange(std::string_view start_date,
                          std::string_view end_date) -> PeriodInsightsData = 0;
  virtual auto QueryWeekly(std::string_view iso_week) -> WeeklyInsightsData = 0;
  virtual auto QueryYearly(std::string_view year) -> YearlyInsightsData = 0;

  virtual auto ListDailyTargets() -> std::vector<std::string> = 0;
  virtual auto ListMonthlyTargets() -> std::vector<std::string> = 0;
  virtual auto ListWeeklyTargets() -> std::vector<std::string> = 0;
  virtual auto ListYearlyTargets() -> std::vector<std::string> = 0;

  virtual auto QueryPeriodBatch(const std::vector<int>& days_list)
      -> std::map<int, PeriodInsightsData> = 0;
  virtual auto QueryAllDaily() -> std::map<std::string, DailyInsightsData> = 0;
  virtual auto QueryAllMonthly()
      -> std::map<std::string, MonthlyInsightsData> = 0;
  virtual auto QueryAllWeekly() -> std::map<std::string, WeeklyInsightsData> = 0;
  virtual auto QueryAllYearly() -> std::map<std::string, YearlyInsightsData> = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_INSIGHTS_DATA_QUERY_SERVICE_H_
