// infrastructure/query/data/orchestrators/report_chart_orchestrator.hpp
#pragma once

#include "infrastructure/sqlite_fwd.hpp"

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"

namespace tracer_core::infrastructure::query::data::orchestrators {

auto HandleReportChartQuery(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput;

}  // namespace tracer_core::infrastructure::query::data::orchestrators

