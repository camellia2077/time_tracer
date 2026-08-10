// infra/insights/insights_service.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_INSIGHTS_SERVICE_H_
#define INFRASTRUCTURE_INSIGHTS_INSIGHTS_SERVICE_H_

#include "infra/sqlite_fwd.hpp"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "application/compat/insights/i_insights_query_service.hpp"
#include "application/ports/insights/i_platform_clock.hpp"
#include "domain/insights/models/period_insights_models.hpp"
#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/shared/interfaces/i_insights_formatter.hpp"

namespace tracer::core::infrastructure::insights {
class InsightsService : public IInsightsQueryService {
 public:
  InsightsService(sqlite3* sqlite_db, const InsightsCatalog& catalog,
                std::shared_ptr<tracer_core::application::ports::IPlatformClock>
                    platform_clock);
  ~InsightsService() override = default;

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
  [[nodiscard]] auto GetOrCreatePeriodFormatter(InsightsFormat format) const
      -> IInsightsFormatter<PeriodInsightsData>&;

  sqlite3* db_;
  const InsightsCatalog& insights_catalog_;
  std::shared_ptr<tracer_core::application::ports::IPlatformClock>
      platform_clock_;
  mutable std::map<InsightsFormat,
                   std::unique_ptr<IInsightsFormatter<PeriodInsightsData>>>
      period_formatter_cache_;
};

}  // namespace tracer::core::infrastructure::insights

namespace infrastructure::insights {

using tracer::core::infrastructure::insights::InsightsService;

}  // namespace infrastructure::insights

using InsightsService = tracer::core::infrastructure::insights::InsightsService;

#endif  // INFRASTRUCTURE_INSIGHTS_INSIGHTS_SERVICE_H_
