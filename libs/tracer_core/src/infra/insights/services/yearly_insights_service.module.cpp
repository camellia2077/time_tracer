#include "infra/sqlite_fwd.hpp"

#include <map>
#include <stdexcept>
#include <string>

#include "infra/insights/services/yearly_insights_service.hpp"
#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/data/queriers/yearly/yearly_querier.hpp"
#include "infra/insights/services/batch_export_helpers.hpp"
#include "infra/insights/shared/factories/generic_formatter_factory.hpp"

import tracer.core.domain.insights.models.query_data_structs;
import tracer.core.domain.insights.types.insights_types;
import tracer.core.shared.period_utils;

namespace modperiod = tracer::core::shared::modperiod;
namespace tracer::core::infrastructure::insights::services {

YearlyInsightsService::YearlyInsightsService(sqlite3* sqlite_db,
                                         const InsightsCatalog& insights_catalog)
    : db_(sqlite_db), insights_catalog_(insights_catalog) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto YearlyInsightsService::GenerateInsights(InsightsFormat format)
    -> FormattedYearlyInsights {
  FormattedYearlyInsights insights;

  ProjectNameCache name_cache =
      ::insights::services::CreateProjectNameCache(db_);

  BatchYearDataFetcher fetcher(db_);
  auto all_years_data = fetcher.FetchAllData();

  auto formatter = GenericFormatterFactory<YearlyInsightsData>::Create(
      format, insights_catalog_);

  ::insights::services::FormatInsightsMap(
      all_years_data, formatter, name_cache,
      [&](const std::string& year_label,
          const std::string& formatted_insights) -> void {
        int gregorian_year = 0;
        if (modperiod::ParseGregorianYear(year_label, gregorian_year)) {
          insights[gregorian_year] = formatted_insights;
        }
      });

  return insights;
}

}  // namespace tracer::core::infrastructure::insights::services
