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
