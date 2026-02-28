// infrastructure/query/data/renderers/data_query_renderer.hpp
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"
#include "infrastructure/query/data/data_query_types.hpp"

namespace tracer_core::infrastructure::query::data::renderers {

[[nodiscard]] auto RenderListOutput(std::string_view action,
                                    const std::vector<std::string>& items,
                                    bool semantic_json) -> std::string;

[[nodiscard]] auto RenderDayDurationsOutput(
    std::string_view action, const std::vector<DayDurationRow>& rows,
    bool semantic_json) -> std::string;

[[nodiscard]] auto RenderDayDurationStatsOutput(
    const std::vector<DayDurationRow>& rows, const DayDurationStats& stats,
    const std::optional<int>& top_n, bool semantic_json) -> std::string;

[[nodiscard]] auto RenderActivitySuggestionsOutput(
    const std::vector<ActivitySuggestionRow>& rows, bool semantic_json)
    -> std::string;

[[nodiscard]] auto RenderProjectTreeOutput(
    const std::vector<ProjectTreeNode>& nodes, int max_depth,
    bool semantic_json) -> std::string;

[[nodiscard]] auto RenderJsonObjectOutput(std::string_view action,
                                          std::string content,
                                          bool semantic_json) -> std::string;

}  // namespace tracer_core::infrastructure::query::data::renderers
