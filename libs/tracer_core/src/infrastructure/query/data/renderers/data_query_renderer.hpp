// infrastructure/query/data/renderers/data_query_renderer.hpp
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"
#include "application/dto/core_requests.hpp"
#include "infrastructure/query/data/data_query_types.hpp"

namespace tracer_core::infrastructure::query::data::renderers {

[[nodiscard]] auto RenderListOutput(std::string_view action,
                                    const std::vector<std::string>& items,
                                    tracer_core::core::dto::DataQueryOutputMode
                                        output_mode) -> std::string;

[[nodiscard]] auto RenderDayDurationsOutput(
    std::string_view action, const std::vector<DayDurationRow>& rows,
    tracer_core::core::dto::DataQueryOutputMode output_mode) -> std::string;

[[nodiscard]] auto RenderDayDurationStatsOutput(
    const std::vector<DayDurationRow>& rows, const DayDurationStats& stats,
    const std::optional<int>& top_n,
    tracer_core::core::dto::DataQueryOutputMode output_mode) -> std::string;

[[nodiscard]] auto RenderActivitySuggestionsOutput(
    const std::vector<ActivitySuggestionRow>& rows,
    tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> std::string;

[[nodiscard]] auto RenderProjectTreeOutput(
    const std::vector<ProjectTreeNode>& nodes, int max_depth,
    tracer_core::core::dto::DataQueryOutputMode output_mode) -> std::string;

[[nodiscard]] auto RenderJsonObjectOutput(std::string_view action,
                                          std::string content,
                                          tracer_core::core::dto::
                                              DataQueryOutputMode output_mode)
    -> std::string;

}  // namespace tracer_core::infrastructure::query::data::renderers
