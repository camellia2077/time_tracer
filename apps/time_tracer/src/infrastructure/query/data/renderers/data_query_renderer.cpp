#include "infrastructure/query/data/renderers/data_query_renderer.hpp"

#include "infrastructure/query/data/renderers/semantic_json_renderer.hpp"
#include "infrastructure/query/data/renderers/text_renderer.hpp"

namespace time_tracer::infrastructure::query::data::renderers {

auto RenderListOutput(std::string_view action,
                      const std::vector<std::string>& items, bool semantic_json)
    -> std::string {
  if (semantic_json) {
    return BuildSemanticListPayload(action, items);
  }
  return RenderListText(items);
}

auto RenderDayDurationsOutput(std::string_view action,
                              const std::vector<DayDurationRow>& rows,
                              bool semantic_json) -> std::string {
  if (semantic_json) {
    return BuildSemanticDayDurationsPayload(action, rows);
  }
  return RenderDayDurationsText(rows);
}

auto RenderDayDurationStatsOutput(const std::vector<DayDurationRow>& rows,
                                  const DayDurationStats& stats,
                                  const std::optional<int>& top_n,
                                  bool semantic_json) -> std::string {
  if (semantic_json) {
    return BuildSemanticDayStatsPayload(rows, stats, top_n);
  }
  return RenderDayDurationStatsOutputText(rows, stats, top_n);
}

auto RenderActivitySuggestionsOutput(
    const std::vector<ActivitySuggestionRow>& rows, bool semantic_json)
    -> std::string {
  if (semantic_json) {
    return BuildSemanticActivitySuggestionsPayload(rows);
  }
  return RenderActivitySuggestionsText(rows);
}

auto RenderProjectTreeOutput(const reporting::ProjectTree& tree, int max_depth,
                             bool semantic_json) -> std::string {
  if (semantic_json) {
    return BuildSemanticTreePayload(tree, max_depth);
  }
  return RenderProjectTreeText(tree, max_depth);
}

auto RenderJsonObjectOutput(std::string_view action, std::string content,
                            bool semantic_json) -> std::string {
  if (semantic_json) {
    return BuildSemanticJsonObjectPayload(action, std::move(content));
  }
  return RenderJsonObjectText(std::move(content));
}

}  // namespace time_tracer::infrastructure::query::data::renderers
