// infrastructure/query/data/orchestrators/days_stats_orchestrator.cpp
#include "infrastructure/query/data/orchestrators/days_stats_orchestrator.hpp"

#include <utility>

#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"
#include "infrastructure/query/data/data_query_repository.hpp"
#include "infrastructure/query/data/renderers/data_query_renderer.hpp"
#include "infrastructure/query/data/stats/day_duration_stats_calculator.hpp"

namespace query_internal =
    infrastructure::persistence::data_query_service_internal;
namespace data_query_renderers =
    tracer_core::infrastructure::query::data::renderers;
namespace data_query_stats = tracer_core::infrastructure::query::data::stats;

namespace tracer_core::infrastructure::query::data::orchestrators {
namespace {

auto BuildSuccessOutput(std::string content)
    -> tracer_core::core::dto::TextOutput {
  return {.ok = true, .content = std::move(content), .error_message = ""};
}

}  // namespace

auto HandleDaysStatsQuery(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    const QueryFilters& base_filters, bool semantic_json)
    -> tracer_core::core::dto::TextOutput {
  auto stats_filters = base_filters;
  if (request.tree_period.has_value()) {
    query_internal::ApplyTreePeriod(request, db_conn, stats_filters);
  }
  stats_filters.limit.reset();
  stats_filters.reverse = false;
  const auto kRows = QueryDayDurations(db_conn, stats_filters);
  const auto kStats = data_query_stats::ComputeDayDurationStats(kRows);
  return BuildSuccessOutput(data_query_renderers::RenderDayDurationStatsOutput(
      kRows, kStats, request.top_n, semantic_json));
}

}  // namespace tracer_core::infrastructure::query::data::orchestrators
