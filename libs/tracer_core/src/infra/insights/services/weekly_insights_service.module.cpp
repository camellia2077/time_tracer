#include "infra/sqlite_fwd.hpp"

#include <map>
#include <stdexcept>
#include <string>

#include "infra/insights/services/weekly_insights_service.hpp"
#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/data/queriers/weekly/weekly_querier.hpp"
#include "infra/insights/services/batch_export_helpers.hpp"
#include "infra/insights/shared/factories/generic_formatter_factory.hpp"

import tracer.core.domain.insights.models.query_data_structs;
import tracer.core.domain.insights.types.insights_types;
import tracer.core.shared.period_utils;

namespace modperiod = tracer::core::shared::modperiod;
namespace tracer::core::infrastructure::insights::services {

WeeklyInsightsService::WeeklyInsightsService(
    sqlite3* database_connection, const InsightsCatalog& insights_catalog)
    : db_(database_connection), insights_catalog_(insights_catalog) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto WeeklyInsightsService::GenerateInsights(InsightsFormat format)
    -> FormattedWeeklyInsights {
  FormattedWeeklyInsights insights;

  ProjectNameCache name_cache =
      ::insights::services::CreateProjectNameCache(db_);

  BatchWeekDataFetcher fetcher(db_);
  auto all_weeks_data = fetcher.FetchAllData();

  auto formatter = GenericFormatterFactory<WeeklyInsightsData>::Create(
      format, insights_catalog_);

  ::insights::services::FormatInsightsMap(
      all_weeks_data, formatter, name_cache,
      [&](const std::string& week_label,
          const std::string& formatted_insights) -> void {
        modperiod::IsoWeek parsed{};
        if (modperiod::ParseIsoWeek(week_label, parsed)) {
          insights[parsed.year][parsed.week] = formatted_insights;
        }
      });

  return insights;
}

}  // namespace tracer::core::infrastructure::insights::services
