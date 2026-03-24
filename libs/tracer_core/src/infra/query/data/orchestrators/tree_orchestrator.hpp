// infra/query/data/orchestrators/tree_orchestrator.hpp
#pragma once

#include "infra/sqlite_fwd.hpp"

#include "application/dto/query_requests.hpp"
#include "application/dto/query_responses.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "infra/query/data/data_query_models.hpp"

namespace tracer::core::infrastructure::query::data::orchestrators {

#include "infra/query/data/orchestrators/detail/tree_orchestrator_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::orchestrators

namespace tracer_core::infrastructure::query::data::orchestrators {

using tracer::core::infrastructure::query::data::orchestrators::
    HandleTreeQuery;

}  // namespace tracer_core::infrastructure::query::data::orchestrators

