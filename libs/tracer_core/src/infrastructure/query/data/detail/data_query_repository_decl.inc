[[nodiscard]] auto QueryYears(sqlite3* db_conn) -> std::vector<std::string>;

[[nodiscard]] auto QueryMonths(sqlite3* db_conn, const std::optional<int>& year)
    -> std::vector<std::string>;

[[nodiscard]] auto QueryDays(sqlite3* db_conn, const std::optional<int>& year,
                             const std::optional<int>& month,
                             const std::optional<std::string>& from_date,
                             const std::optional<std::string>& to_date,
                             bool reverse, const std::optional<int>& limit)
    -> std::vector<std::string>;

[[nodiscard]] auto QueryDatesByFilters(sqlite3* db_conn,
                                       const QueryFilters& filters)
    -> std::vector<std::string>;

[[nodiscard]] auto QueryDayDurations(sqlite3* db_conn,
                                     const QueryFilters& filters)
    -> std::vector<DayDurationRow>;

[[nodiscard]] auto QueryProjectRootNames(sqlite3* db_conn)
    -> std::vector<std::string>;

[[nodiscard]] auto QueryDayDurationsByRootInDateRange(
    sqlite3* db_conn, const std::optional<std::string>& root,
    std::string_view from_date, std::string_view to_date)
    -> std::vector<DayDurationRow>;

[[nodiscard]] auto QueryActivitySuggestions(
    sqlite3* db_conn, const ActivitySuggestionQueryOptions& options)
    -> std::vector<ActivitySuggestionRow>;

[[nodiscard]] auto QueryLatestTrackedDate(sqlite3* db_conn)
    -> std::optional<std::string>;

[[nodiscard]] auto QueryProjectTree(sqlite3* db_conn,
                                    const QueryFilters& filters)
    -> reporting::ProjectTree;
