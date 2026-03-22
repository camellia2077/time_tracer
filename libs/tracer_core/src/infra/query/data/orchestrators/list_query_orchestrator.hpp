// infra/query/data/orchestrators/list_query_orchestrator.hpp
#pragma once

#include "infra/sqlite_fwd.hpp"

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "infra/query/data/data_query_models.hpp"

namespace tracer::core::infrastructure::query::data::orchestrators {

#include "infra/query/data/orchestrators/detail/list_query_orchestrator_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::orchestrators

namespace tracer_core::infrastructure::query::data::orchestrators {

using tracer::core::infrastructure::query::data::orchestrators::
    HandleActivitySuggestQuery;
using tracer::core::infrastructure::query::data::orchestrators::
    HandleDaysDurationQuery;
using tracer::core::infrastructure::query::data::orchestrators::
    HandleDaysQuery;
using tracer::core::infrastructure::query::data::orchestrators::
    HandleMonthsQuery;
using tracer::core::infrastructure::query::data::orchestrators::
    HandleSearchQuery;
using tracer::core::infrastructure::query::data::orchestrators::
    HandleYearsQuery;

}  // namespace tracer_core::infrastructure::query::data::orchestrators

