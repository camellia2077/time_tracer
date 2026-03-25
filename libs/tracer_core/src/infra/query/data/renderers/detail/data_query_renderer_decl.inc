[[nodiscard]] auto RenderListOutput(
    std::string_view action, const std::vector<std::string>& items,
    tracer_core::core::dto::DataQueryOutputMode output_mode) -> std::string;

[[nodiscard]] auto RenderDayDurationsOutput(
    std::string_view action, const std::vector<DayDurationRow>& rows,
    tracer_core::core::dto::DataQueryOutputMode output_mode) -> std::string;

[[nodiscard]] auto RenderDayDurationStatsOutput(
    const std::vector<DayDurationRow>& rows, const DayDurationStats& stats,
    const std::optional<int>& top_n,
    tracer_core::core::dto::DataQueryOutputMode output_mode) -> std::string;

[[nodiscard]] auto RenderActivitySuggestionsOutput(
    const std::vector<ActivitySuggestionRow>& rows,
    tracer_core::core::dto::DataQueryOutputMode output_mode) -> std::string;

[[nodiscard]] auto RenderProjectTreeOutput(
    const std::vector<ProjectTreeNode>& nodes, int max_depth,
    tracer_core::core::dto::DataQueryOutputMode output_mode) -> std::string;

[[nodiscard]] auto RenderJsonObjectOutput(
    std::string_view action, std::string content,
    tracer_core::core::dto::DataQueryOutputMode output_mode) -> std::string;
