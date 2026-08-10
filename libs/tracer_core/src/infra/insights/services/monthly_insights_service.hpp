// infra/insights/services/monthly_insights_service.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_SERVICES_MONTHLY_INSIGHTS_SERVICE_H_
#define INFRASTRUCTURE_INSIGHTS_SERVICES_MONTHLY_INSIGHTS_SERVICE_H_

#include "infra/sqlite_fwd.hpp"

#include <map>
#include <string>

#include "domain/insights/models/query_data_structs.hpp"
#include "domain/insights/types/insights_types.hpp"
#include "infra/config/models/insights_catalog.hpp"

namespace tracer::core::infrastructure::insights::services {
class MonthlyInsightsService {
 public:
  explicit MonthlyInsightsService(sqlite3* database_connection,
                                const InsightsCatalog& insights_catalog);

  [[nodiscard]] auto GenerateInsights(InsightsFormat format)
      -> FormattedMonthlyInsights;

 private:
  sqlite3* db_;
  const InsightsCatalog& insights_catalog_;
};

}  // namespace tracer::core::infrastructure::insights::services

namespace infrastructure::insights::services {

using tracer::core::infrastructure::insights::services::MonthlyInsightsService;

}  // namespace infrastructure::insights::services

using MonthlyInsightsService =
    tracer::core::infrastructure::insights::services::MonthlyInsightsService;

#endif  // INFRASTRUCTURE_INSIGHTS_SERVICES_MONTHLY_INSIGHTS_SERVICE_H_
