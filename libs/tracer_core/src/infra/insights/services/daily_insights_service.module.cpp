#include "infra/sqlite_fwd.hpp"

#include <stdexcept>
#include <string>

#include "infra/insights/services/daily_insights_service.hpp"
#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/data/queriers/daily/daily_querier.hpp"
#include "infra/insights/services/batch_export_helpers.hpp"
#include "infra/insights/shared/factories/generic_formatter_factory.hpp"

import tracer.core.domain.insights.models.query_data_structs;
import tracer.core.domain.insights.types.insights_types;

namespace tracer::core::infrastructure::insights::services {

DailyInsightsService::DailyInsightsService(sqlite3* sqlite_db,
                                       const InsightsCatalog& insights_catalog)
    : db_(sqlite_db), insights_catalog_(insights_catalog) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto DailyInsightsService::GenerateAllInsights(InsightsFormat format)
    -> FormattedGroupedInsights {
  FormattedGroupedInsights grouped_insights;

  ProjectNameCache name_cache =
      ::insights::services::CreateProjectNameCache(db_);

  BatchDayDataFetcher fetcher(db_, name_cache, &insights_catalog_.statuses.day);
  BatchDataResult batch_data = fetcher.FetchAllData();

  auto formatter =
      GenericFormatterFactory<DailyInsightsData>::Create(format, insights_catalog_);

  for (const auto& [date, year, month] : batch_data.date_order) {
    DailyInsightsData& data = batch_data.data_map[date];

    if (data.total_duration > 0) {
      ::insights::services::EnsureProjectTree(data, name_cache);

      std::string formatted_insights = formatter->FormatInsights(data);
      grouped_insights[year][month].push_back({date, formatted_insights});
    }
  }

  return grouped_insights;
}

}  // namespace tracer::core::infrastructure::insights::services
