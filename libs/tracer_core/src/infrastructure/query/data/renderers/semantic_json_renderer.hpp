// infrastructure/query/data/renderers/semantic_json_renderer.hpp
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"
#include "infrastructure/query/data/data_query_types.hpp"

namespace tracer::core::infrastructure::query::data::renderers {

#include "infrastructure/query/data/renderers/detail/semantic_json_renderer_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::renderers

namespace tracer_core::infrastructure::query::data::renderers {

using tracer::core::infrastructure::query::data::renderers::
    BuildSemanticActivitySuggestionsPayload;
using tracer::core::infrastructure::query::data::renderers::
    BuildSemanticDayDurationsPayload;
using tracer::core::infrastructure::query::data::renderers::
    BuildSemanticDayStatsPayload;
using tracer::core::infrastructure::query::data::renderers::
    BuildSemanticJsonObjectPayload;
using tracer::core::infrastructure::query::data::renderers::
    BuildSemanticListPayload;
using tracer::core::infrastructure::query::data::renderers::
    BuildSemanticTreePayload;

}  // namespace tracer_core::infrastructure::query::data::renderers
