#pragma once

#include <sqlite3.h>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "infrastructure/query/data/data_query_models.hpp"

namespace time_tracer::infrastructure::query::data::orchestrators {

auto HandleYearsQuery(sqlite3* db_conn, bool semantic_json)
    -> time_tracer::core::dto::TextOutput;

auto HandleMonthsQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                       bool semantic_json)
    -> time_tracer::core::dto::TextOutput;

auto HandleDaysQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                     bool semantic_json) -> time_tracer::core::dto::TextOutput;

auto HandleDaysDurationQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                             bool semantic_json)
    -> time_tracer::core::dto::TextOutput;

auto HandleSearchQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                       bool semantic_json)
    -> time_tracer::core::dto::TextOutput;

auto HandleActivitySuggestQuery(
    sqlite3* db_conn, const time_tracer::core::dto::DataQueryRequest& request,
    bool semantic_json) -> time_tracer::core::dto::TextOutput;

}  // namespace time_tracer::infrastructure::query::data::orchestrators
