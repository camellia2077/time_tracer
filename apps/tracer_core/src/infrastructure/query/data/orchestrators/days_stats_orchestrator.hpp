// infrastructure/query/data/orchestrators/days_stats_orchestrator.hpp
#pragma once

#include <sqlite3.h>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "infrastructure/query/data/data_query_models.hpp"

namespace tracer_core::infrastructure::query::data::orchestrators {

auto HandleDaysStatsQuery(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    const QueryFilters& base_filters, bool semantic_json)
    -> tracer_core::core::dto::TextOutput;

}  // namespace tracer_core::infrastructure::query::data::orchestrators
