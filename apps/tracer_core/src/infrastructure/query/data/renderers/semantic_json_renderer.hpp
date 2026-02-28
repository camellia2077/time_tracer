// infrastructure/query/data/renderers/semantic_json_renderer.hpp
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"
#include "infrastructure/query/data/data_query_types.hpp"

namespace tracer_core::infrastructure::query::data::renderers {

[[nodiscard]] auto BuildSemanticListPayload(
    std::string_view action, const std::vector<std::string>& items)
    -> std::string;

[[nodiscard]] auto BuildSemanticDayDurationsPayload(
    std::string_view action, const std::vector<DayDurationRow>& rows)
    -> std::string;

[[nodiscard]] auto BuildSemanticDayStatsPayload(
    const std::vector<DayDurationRow>& rows, const DayDurationStats& stats,
    const std::optional<int>& top_n) -> std::string;

[[nodiscard]] auto BuildSemanticActivitySuggestionsPayload(
    const std::vector<ActivitySuggestionRow>& rows) -> std::string;

[[nodiscard]] auto BuildSemanticTreePayload(
    const std::vector<ProjectTreeNode>& nodes, int max_depth) -> std::string;

[[nodiscard]] auto BuildSemanticJsonObjectPayload(std::string_view action,
                                                  std::string content)
    -> std::string;

}  // namespace tracer_core::infrastructure::query::data::renderers
