// infrastructure/query/data/renderers/data_query_renderer.cpp
#include "infrastructure/query/data/renderers/data_query_renderer.hpp"

#include "infrastructure/query/data/renderers/semantic_json_renderer.hpp"
#include "infrastructure/query/data/renderers/text_renderer.hpp"

namespace tracer_core::infrastructure::query::data::renderers {
namespace {

using tracer_core::core::dto::DataQueryOutputMode;

[[nodiscard]] auto ShouldRenderSemanticJson(DataQueryOutputMode output_mode)
    -> bool {
  return output_mode == DataQueryOutputMode::kSemanticJson;
}

}  // namespace

auto RenderListOutput(std::string_view action,
                      const std::vector<std::string>& items,
                      DataQueryOutputMode output_mode)
    -> std::string {
  if (ShouldRenderSemanticJson(output_mode)) {
    return BuildSemanticListPayload(action, items);
  }
  return RenderListText(items);
}

auto RenderDayDurationsOutput(std::string_view action,
                              const std::vector<DayDurationRow>& rows,
                              DataQueryOutputMode output_mode) -> std::string {
  if (ShouldRenderSemanticJson(output_mode)) {
    return BuildSemanticDayDurationsPayload(action, rows);
  }
  return RenderDayDurationsText(rows);
}

auto RenderDayDurationStatsOutput(const std::vector<DayDurationRow>& rows,
                                  const DayDurationStats& stats,
                                  const std::optional<int>& top_n,
                                  DataQueryOutputMode output_mode)
    -> std::string {
  if (ShouldRenderSemanticJson(output_mode)) {
    return BuildSemanticDayStatsPayload(rows, stats, top_n);
  }
  return RenderDayDurationStatsOutputText(rows, stats, top_n);
}

auto RenderActivitySuggestionsOutput(
    const std::vector<ActivitySuggestionRow>& rows,
    DataQueryOutputMode output_mode)
    -> std::string {
  if (ShouldRenderSemanticJson(output_mode)) {
    return BuildSemanticActivitySuggestionsPayload(rows);
  }
  return RenderActivitySuggestionsText(rows);
}

auto RenderProjectTreeOutput(const std::vector<ProjectTreeNode>& nodes,
                             int max_depth, DataQueryOutputMode output_mode)
    -> std::string {
  if (ShouldRenderSemanticJson(output_mode)) {
    return BuildSemanticTreePayload(nodes, max_depth);
  }
  return RenderProjectTreeText(nodes, max_depth);
}

auto RenderJsonObjectOutput(std::string_view action, std::string content,
                            DataQueryOutputMode output_mode) -> std::string {
  if (ShouldRenderSemanticJson(output_mode)) {
    return BuildSemanticJsonObjectPayload(action, std::move(content));
  }
  return RenderJsonObjectText(std::move(content));
}

}  // namespace tracer_core::infrastructure::query::data::renderers
