// infrastructure/query/data/orchestrators/tree_orchestrator.hpp
#pragma once

#include "infrastructure/sqlite_fwd.hpp"

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "infrastructure/query/data/data_query_models.hpp"

namespace tracer_core::infrastructure::query::data::orchestrators {

auto HandleTreeQuery(sqlite3* db_conn,
                     const tracer_core::core::dto::DataQueryRequest& request,
                     const QueryFilters& base_filters,
                     tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput;

}  // namespace tracer_core::infrastructure::query::data::orchestrators

