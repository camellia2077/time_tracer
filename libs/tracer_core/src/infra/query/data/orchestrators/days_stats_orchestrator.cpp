// infra/query/data/orchestrators/days_stats_orchestrator.cpp
#include "infra/query/data/orchestrators/days_stats_orchestrator.hpp"

#include <utility>

import tracer.core.infrastructure.query.data.internal.period;
import tracer.core.infrastructure.query.data.repository;
import tracer.core.infrastructure.query.data.renderers;
import tracer.core.infrastructure.query.data.stats;

namespace query_internal =
    tracer::core::infrastructure::query::data::internal;
namespace data_query_repository = tracer::core::infrastructure::query::data;
namespace data_query_renderers =
    tracer::core::infrastructure::query::data::renderers;
namespace data_query_stats = tracer::core::infrastructure::query::data::stats;

namespace tracer::core::infrastructure::query::data::orchestrators {
namespace {

auto BuildSuccessOutput(std::string content)
    -> tracer_core::core::dto::TextOutput {
  return {.ok = true, .content = std::move(content), .error_message = ""};
}

}  // namespace

auto HandleDaysStatsQuery(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    const QueryFilters& base_filters,
    tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput {
  auto stats_filters = base_filters;
  if (request.tree_period.has_value()) {
    query_internal::ApplyTreePeriod(request, db_conn, stats_filters);
  }
  stats_filters.limit.reset();
  stats_filters.reverse = false;
  const auto kRows =
      data_query_repository::QueryDayDurations(db_conn, stats_filters);
  const auto kStats = data_query_stats::ComputeDayDurationStats(kRows);
  return BuildSuccessOutput(data_query_renderers::RenderDayDurationStatsOutput(
      kRows, kStats, request.top_n, output_mode));
}

}  // namespace tracer::core::infrastructure::query::data::orchestrators
