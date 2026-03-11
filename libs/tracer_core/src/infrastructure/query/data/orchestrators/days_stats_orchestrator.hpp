// infrastructure/query/data/orchestrators/days_stats_orchestrator.hpp
#pragma once

#include "infrastructure/sqlite_fwd.hpp"

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "infrastructure/query/data/data_query_models.hpp"

namespace tracer::core::infrastructure::query::data::orchestrators {

#include "infrastructure/query/data/orchestrators/detail/days_stats_orchestrator_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::orchestrators

namespace tracer_core::infrastructure::query::data::orchestrators {

using tracer::core::infrastructure::query::data::orchestrators::
    HandleDaysStatsQuery;

}  // namespace tracer_core::infrastructure::query::data::orchestrators

