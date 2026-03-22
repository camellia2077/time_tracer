module;

#include "infra/query/data/renderers/data_query_renderer.hpp"

export module tracer.core.infrastructure.query.data.renderers.data_query_renderer;

export namespace tracer::core::infrastructure::query::data::renderers {

using ::tracer::core::infrastructure::query::data::renderers::
    RenderActivitySuggestionsOutput;
using ::tracer::core::infrastructure::query::data::renderers::
    RenderDayDurationsOutput;
using ::tracer::core::infrastructure::query::data::renderers::
    RenderDayDurationStatsOutput;
using ::tracer::core::infrastructure::query::data::renderers::
    RenderJsonObjectOutput;
using ::tracer::core::infrastructure::query::data::renderers::RenderListOutput;
using ::tracer::core::infrastructure::query::data::renderers::
    RenderProjectTreeOutput;

}  // namespace tracer::core::infrastructure::query::data::renderers
