// infra/insights/services/yearly_insights_service.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_SERVICES_YEARLY_INSIGHTS_SERVICE_H_
#define INFRASTRUCTURE_INSIGHTS_SERVICES_YEARLY_INSIGHTS_SERVICE_H_

#include "infra/sqlite_fwd.hpp"

#include <map>
#include <string>

#include "domain/insights/models/query_data_structs.hpp"
#include "domain/insights/types/insights_types.hpp"
#include "infra/config/models/insights_catalog.hpp"

namespace tracer::core::infrastructure::insights::services {
class YearlyInsightsService {
 public:
  explicit YearlyInsightsService(sqlite3* sqlite_db,
                                 const InsightsCatalog& insights_catalog);

  auto GenerateInsights(InsightsFormat format) -> FormattedYearlyInsights;

 private:
  sqlite3* db_;
  const InsightsCatalog& insights_catalog_;
};

}  // namespace tracer::core::infrastructure::insights::services

namespace infrastructure::insights::services {

using tracer::core::infrastructure::insights::services::YearlyInsightsService;

}  // namespace infrastructure::insights::services

using YearlyInsightsService =
    tracer::core::infrastructure::insights::services::YearlyInsightsService;

#endif  // INFRASTRUCTURE_INSIGHTS_SERVICES_YEARLY_INSIGHTS_SERVICE_H_
