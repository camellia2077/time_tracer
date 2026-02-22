#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "infrastructure/query/data/data_query_models.hpp"
#include "infrastructure/query/data/data_query_repository_sql.hpp"

struct sqlite3;

namespace time_tracer::infrastructure::query::data::internal {

struct DateRangeBounds {
  std::string_view from_date;
  std::string_view to_date;
};

[[nodiscard]] auto ClampPositiveOrDefault(int value, int fallback) -> int;

[[nodiscard]] auto BuildRootPrefixLikePattern(std::string_view root)
    -> std::string;

auto EnsureProjectPathSnapshotColumnOrThrow(sqlite3* db_conn,
                                            std::string_view query_name)
    -> void;

[[nodiscard]] auto BuildYearsSql() -> std::string;

[[nodiscard]] auto BuildMonthsSql(const std::optional<int>& year,
                                  std::vector<detail::SqlParam>& params)
    -> std::string;

[[nodiscard]] auto BuildDaysSql(const std::optional<int>& year,
                                const std::optional<int>& month,
                                const std::optional<std::string>& from_date,
                                const std::optional<std::string>& to_date,
                                bool reverse, const std::optional<int>& limit,
                                std::vector<detail::SqlParam>& params)
    -> std::string;

[[nodiscard]] auto BuildDatesByFiltersSql(sqlite3* db_conn,
                                          const QueryFilters& filters,
                                          std::vector<detail::SqlParam>& params)
    -> std::string;

[[nodiscard]] auto BuildDayDurationsSql(sqlite3* db_conn,
                                        const QueryFilters& filters,
                                        std::vector<detail::SqlParam>& params)
    -> std::string;

[[nodiscard]] auto BuildProjectRootNamesSql(sqlite3* db_conn) -> std::string;

[[nodiscard]] auto BuildDayDurationsByRootInDateRangeSql(
    sqlite3* db_conn, const std::optional<std::string>& root,
    const DateRangeBounds& date_range, std::vector<detail::SqlParam>& params)
    -> std::string;

[[nodiscard]] auto BuildActivitySuggestionsSql(
    const ActivitySuggestionQueryOptions& options) -> std::string;

[[nodiscard]] auto BuildLatestTrackedDateSql() -> std::string;

[[nodiscard]] auto BuildProjectTreeSql(sqlite3* db_conn,
                                       const QueryFilters& filters,
                                       std::vector<detail::SqlParam>& params)
    -> std::string;

[[nodiscard]] auto ExecuteActivitySuggestions(
    sqlite3* db_conn, const std::string& sql,
    const ActivitySuggestionQueryOptions& options, int lookback_days, int limit)
    -> std::vector<ActivitySuggestionRow>;

[[nodiscard]] auto ExecuteProjectTreeRecords(
    sqlite3* db_conn, const std::string& sql,
    const std::vector<detail::SqlParam>& params)
    -> std::vector<std::pair<std::string, long long>>;

}  // namespace time_tracer::infrastructure::query::data::internal
