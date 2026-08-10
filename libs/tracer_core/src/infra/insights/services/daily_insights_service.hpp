// infra/insights/services/daily_insights_service.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_SERVICES_DAILY_INSIGHTS_SERVICE_H_
#define INFRASTRUCTURE_INSIGHTS_SERVICES_DAILY_INSIGHTS_SERVICE_H_

#include "infra/sqlite_fwd.hpp"

#include <string>

#include "domain/insights/models/query_data_structs.hpp"
#include "domain/insights/types/insights_types.hpp"
#include "infra/config/models/insights_catalog.hpp"

namespace tracer::core::infrastructure::insights::services {
class DailyInsightsService {
 public:
  explicit DailyInsightsService(sqlite3* sqlite_db,
                              const InsightsCatalog& insights_catalog);

  auto GenerateAllInsights(InsightsFormat format) -> FormattedGroupedInsights;

 private:
  sqlite3* db_;
  const InsightsCatalog& insights_catalog_;
};

}  // namespace tracer::core::infrastructure::insights::services

namespace infrastructure::insights::services {

using tracer::core::infrastructure::insights::services::DailyInsightsService;

}  // namespace infrastructure::insights::services

using DailyInsightsService =
    tracer::core::infrastructure::insights::services::DailyInsightsService;

#endif  // INFRASTRUCTURE_INSIGHTS_SERVICES_DAILY_INSIGHTS_SERVICE_H_
