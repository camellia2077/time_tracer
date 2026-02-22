#pragma once

#include <sqlite3.h>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"

namespace time_tracer::infrastructure::query::data::orchestrators {

auto HandleReportChartQuery(
    sqlite3* db_conn, const time_tracer::core::dto::DataQueryRequest& request,
    bool semantic_json) -> time_tracer::core::dto::TextOutput;

}  // namespace time_tracer::infrastructure::query::data::orchestrators
