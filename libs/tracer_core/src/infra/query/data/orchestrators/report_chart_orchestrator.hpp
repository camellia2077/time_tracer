// infra/query/data/orchestrators/report_chart_orchestrator.hpp
#pragma once

#include "infra/sqlite_fwd.hpp"

#include "application/dto/query_requests.hpp"
#include "application/dto/shared_envelopes.hpp"

namespace tracer::core::infrastructure::query::data::orchestrators {

#include "infra/query/data/orchestrators/detail/report_chart_orchestrator_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::orchestrators

namespace tracer_core::infrastructure::query::data::orchestrators {

using tracer::core::infrastructure::query::data::orchestrators::
    HandleReportChartQuery;

}  // namespace tracer_core::infrastructure::query::data::orchestrators
