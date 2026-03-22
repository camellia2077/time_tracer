// infra/query/data/renderers/data_query_renderer.hpp
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"
#include "application/dto/core_requests.hpp"
#include "infra/query/data/data_query_types.hpp"

namespace tracer::core::infrastructure::query::data::renderers {

#include "infra/query/data/renderers/detail/data_query_renderer_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::renderers

namespace tracer_core::infrastructure::query::data::renderers {

using tracer::core::infrastructure::query::data::renderers::
    RenderActivitySuggestionsOutput;
using tracer::core::infrastructure::query::data::renderers::
    RenderDayDurationsOutput;
using tracer::core::infrastructure::query::data::renderers::
    RenderDayDurationStatsOutput;
using tracer::core::infrastructure::query::data::renderers::
    RenderJsonObjectOutput;
using tracer::core::infrastructure::query::data::renderers::RenderListOutput;
using tracer::core::infrastructure::query::data::renderers::
    RenderProjectTreeOutput;

}  // namespace tracer_core::infrastructure::query::data::renderers
