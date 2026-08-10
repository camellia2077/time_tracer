#ifndef INFRASTRUCTURE_INSIGHTS_LAZY_SQLITE_INSIGHTS_QUERY_SERVICE_H_
#define INFRASTRUCTURE_INSIGHTS_LAZY_SQLITE_INSIGHTS_QUERY_SERVICE_H_

#include <filesystem>
#include <memory>

#include "application/compat/insights/i_insights_query_service.hpp"
#include "application/ports/insights/i_platform_clock.hpp"
#include "infra/config/models/insights_catalog.hpp"

namespace tracer::core::infrastructure::insights {
class LazySqliteInsightsQueryService final : public IInsightsQueryService {
 public:
  LazySqliteInsightsQueryService(
      std::filesystem::path db_path,
      std::shared_ptr<InsightsCatalog> insights_catalog,
      std::shared_ptr<tracer_core::application::ports::IPlatformClock>
          platform_clock);

  [[nodiscard]] auto RunDailyQuery(std::string_view date_str,
                                   InsightsFormat format) const
      -> std::string override;
  [[nodiscard]] auto RunPeriodQuery(int days, InsightsFormat format) const
      -> std::string override;
  [[nodiscard]] auto RunMonthlyQuery(std::string_view year_month_str,
                                     InsightsFormat format) const
      -> std::string override;
  [[nodiscard]] auto RunWeeklyQuery(std::string_view iso_week_str,
                                    InsightsFormat format) const
      -> std::string override;
  [[nodiscard]] auto RunYearlyQuery(std::string_view year_str,
                                    InsightsFormat format) const
      -> std::string override;

 private:
  std::filesystem::path db_path_;
  std::shared_ptr<InsightsCatalog> insights_catalog_;
  std::shared_ptr<tracer_core::application::ports::IPlatformClock>
      platform_clock_;
};

}  // namespace tracer::core::infrastructure::insights

namespace infrastructure::insights {

using tracer::core::infrastructure::insights::LazySqliteInsightsQueryService;

}  // namespace infrastructure::insights

#endif  // INFRASTRUCTURE_INSIGHTS_LAZY_SQLITE_INSIGHTS_QUERY_SERVICE_H_
