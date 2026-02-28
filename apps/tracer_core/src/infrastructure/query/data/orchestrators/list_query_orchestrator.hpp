// infrastructure/query/data/orchestrators/list_query_orchestrator.hpp
#pragma once

#include <sqlite3.h>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "infrastructure/query/data/data_query_models.hpp"

namespace tracer_core::infrastructure::query::data::orchestrators {

auto HandleYearsQuery(sqlite3* db_conn, bool semantic_json)
    -> tracer_core::core::dto::TextOutput;

auto HandleMonthsQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                       bool semantic_json)
    -> tracer_core::core::dto::TextOutput;

auto HandleDaysQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                     bool semantic_json) -> tracer_core::core::dto::TextOutput;

auto HandleDaysDurationQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                             bool semantic_json)
    -> tracer_core::core::dto::TextOutput;

auto HandleSearchQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                       bool semantic_json)
    -> tracer_core::core::dto::TextOutput;

auto HandleActivitySuggestQuery(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    bool semantic_json) -> tracer_core::core::dto::TextOutput;

}  // namespace tracer_core::infrastructure::query::data::orchestrators
