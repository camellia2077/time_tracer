module;

#include "infrastructure/query/data/renderers/text_renderer.hpp"

export module tracer.core.infrastructure.query.data.renderers.text_renderer;

export namespace tracer::core::infrastructure::query::data::renderers {

using ::tracer::core::infrastructure::query::data::renderers::
    RenderActivitySuggestionsText;
using ::tracer::core::infrastructure::query::data::renderers::
    RenderDayDurationsText;
using ::tracer::core::infrastructure::query::data::renderers::
    RenderDayDurationStatsOutputText;
using ::tracer::core::infrastructure::query::data::renderers::
    RenderDayDurationStatsText;
using ::tracer::core::infrastructure::query::data::renderers::RenderJsonObjectText;
using ::tracer::core::infrastructure::query::data::renderers::RenderListText;
using ::tracer::core::infrastructure::query::data::renderers::RenderProjectTreeText;
using ::tracer::core::infrastructure::query::data::renderers::
    RenderTopDayDurationsText;

}  // namespace tracer::core::infrastructure::query::data::renderers
