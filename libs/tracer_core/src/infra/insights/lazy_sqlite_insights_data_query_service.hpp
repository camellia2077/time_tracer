#ifndef INFRASTRUCTURE_INSIGHTS_LAZY_SQLITE_INSIGHTS_DATA_QUERY_SERVICE_H_
#define INFRASTRUCTURE_INSIGHTS_LAZY_SQLITE_INSIGHTS_DATA_QUERY_SERVICE_H_

#include <filesystem>
#include <memory>

#include "application/ports/insights/i_platform_clock.hpp"
#include "application/ports/insights/i_insights_data_query_service.hpp"
#include "infra/config/models/insights_catalog.hpp"

namespace tracer::core::infrastructure::insights {
class LazySqliteInsightsDataQueryService final
    : public tracer_core::application::ports::IInsightsDataQueryService {
 public:
  LazySqliteInsightsDataQueryService(
      std::filesystem::path db_path,
      std::shared_ptr<tracer_core::application::ports::IPlatformClock>
          platform_clock,
      std::shared_ptr<const InsightsCatalog> insights_catalog = nullptr);

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
  std::filesystem::path db_path_;
  std::shared_ptr<tracer_core::application::ports::IPlatformClock>
      platform_clock_;
  std::shared_ptr<const InsightsCatalog> insights_catalog_;
};

}  // namespace tracer::core::infrastructure::insights

namespace infrastructure::insights {

using tracer::core::infrastructure::insights::LazySqliteInsightsDataQueryService;

}  // namespace infrastructure::insights

#endif  // INFRASTRUCTURE_INSIGHTS_LAZY_SQLITE_INSIGHTS_DATA_QUERY_SERVICE_H_
