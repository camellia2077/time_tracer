#include "infra/sqlite_fwd.hpp"

#include <map>
#include <stdexcept>
#include <string>
#include <utility>

#include "infra/insights/services/monthly_insights_service.hpp"
#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/data/queriers/monthly/monthly_querier.hpp"
#include "infra/insights/services/batch_export_helpers.hpp"
#include "infra/insights/shared/factories/generic_formatter_factory.hpp"

import tracer.core.domain.insights.models.query_data_structs;
import tracer.core.domain.insights.types.insights_types;

namespace {
constexpr size_t kYearMonthStrMinLen = 7;
constexpr size_t kYearMonthYearLen = 4;
constexpr size_t kYearMonthDashPos = 4;
constexpr size_t kYearMonthMonthPos = 5;
constexpr size_t kYearMonthMonthLen = 2;
}  // namespace

static auto ParseYearMonth(const std::string& year_month_str)
    -> std::pair<int, int> {
  if (year_month_str.size() >= kYearMonthStrMinLen &&
      year_month_str[kYearMonthDashPos] == '-') {
    try {
      int year = std::stoi(year_month_str.substr(0, kYearMonthYearLen));
      int month = std::stoi(
          year_month_str.substr(kYearMonthMonthPos, kYearMonthMonthLen));
      return {year, month};
    } catch (...) {
      return {0, 0};
    }
  }
  return {0, 0};
}

namespace tracer::core::infrastructure::insights::services {

MonthlyInsightsService::MonthlyInsightsService(sqlite3* database_connection,
                                           const InsightsCatalog& insights_catalog)
    : db_(database_connection), insights_catalog_(insights_catalog) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto MonthlyInsightsService::GenerateInsights(InsightsFormat format)
    -> FormattedMonthlyInsights {
  FormattedMonthlyInsights insights;

  ProjectNameCache name_cache =
      ::insights::services::CreateProjectNameCache(db_);

  BatchMonthDataFetcher fetcher(db_);
  auto all_months_data = fetcher.FetchAllData();

  auto formatter = GenericFormatterFactory<MonthlyInsightsData>::Create(
      format, insights_catalog_);

  ::insights::services::FormatInsightsMap(
      all_months_data, formatter, name_cache,
      [&](const std::string& year_month_str,
          const std::string& formatted_insights) -> void {
        auto [year, month] = ParseYearMonth(year_month_str);
        if (year > 0 && month > 0) {
          insights[year][month] = formatted_insights;
        }
      });

  return insights;
}

}  // namespace tracer::core::infrastructure::insights::services
