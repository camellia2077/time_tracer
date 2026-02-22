#pragma once

#include <optional>
#include <string>
#include <vector>

#include "domain/reports/models/project_tree.hpp"
#include "infrastructure/query/data/data_query_types.hpp"

namespace time_tracer::infrastructure::query::data::renderers {

[[nodiscard]] auto RenderListText(const std::vector<std::string>& items)
    -> std::string;

[[nodiscard]] auto RenderDayDurationsText(
    const std::vector<DayDurationRow>& rows) -> std::string;

[[nodiscard]] auto RenderDayDurationStatsText(const DayDurationStats& stats)
    -> std::string;

[[nodiscard]] auto RenderTopDayDurationsText(
    const std::vector<DayDurationRow>& rows, int top_n) -> std::string;

[[nodiscard]] auto RenderDayDurationStatsOutputText(
    const std::vector<DayDurationRow>& rows, const DayDurationStats& stats,
    const std::optional<int>& top_n) -> std::string;

[[nodiscard]] auto RenderActivitySuggestionsText(
    const std::vector<ActivitySuggestionRow>& rows) -> std::string;

[[nodiscard]] auto RenderProjectTreeText(const reporting::ProjectTree& tree,
                                         int max_depth) -> std::string;

[[nodiscard]] auto RenderJsonObjectText(std::string content) -> std::string;

}  // namespace time_tracer::infrastructure::query::data::renderers
