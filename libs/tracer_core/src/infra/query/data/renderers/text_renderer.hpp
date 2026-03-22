// infra/query/data/renderers/text_renderer.hpp
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"
#include "infra/query/data/data_query_types.hpp"

namespace tracer::core::infrastructure::query::data::renderers {

#include "infra/query/data/renderers/detail/text_renderer_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::renderers

namespace tracer_core::infrastructure::query::data::renderers {

using tracer::core::infrastructure::query::data::renderers::
    RenderActivitySuggestionsText;
using tracer::core::infrastructure::query::data::renderers::
    RenderDayDurationsText;
using tracer::core::infrastructure::query::data::renderers::
    RenderDayDurationStatsOutputText;
using tracer::core::infrastructure::query::data::renderers::
    RenderDayDurationStatsText;
using tracer::core::infrastructure::query::data::renderers::RenderJsonObjectText;
using tracer::core::infrastructure::query::data::renderers::RenderListText;
using tracer::core::infrastructure::query::data::renderers::RenderProjectTreeText;
using tracer::core::infrastructure::query::data::renderers::
    RenderTopDayDurationsText;

}  // namespace tracer_core::infrastructure::query::data::renderers
