// infra/query/data/data_query_repository.cpp
#include "infra/query/data/data_query_repository.hpp"

#include <cstdint>
#include <sqlite3.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "infra/query/data/internal/project_tree_projection.hpp"
#include "infra/query/data/data_query_repository_internal.hpp"
#include "infra/query/data/data_query_repository_sql.hpp"

namespace query_data_detail = tracer_core::infrastructure::query::data::detail;
namespace query_data_internal =
    tracer_core::infrastructure::query::data::internal;

namespace tracer::core::infrastructure::query::data {
namespace {

constexpr int kFirstDayOfMonth = 1;
constexpr int kDefaultLookbackDays = 10;
constexpr int kDefaultSuggestLimit = 5;
constexpr int kYearMonthLength = 7;

}  // namespace

auto QueryYears(sqlite3* db_conn) -> std::vector<std::string> {
  const std::string kSql = query_data_internal::BuildYearsSql();
  return query_data_detail::QueryStringColumn(db_conn, kSql, {});
}

auto QueryMonths(sqlite3* db_conn, const std::optional<int>& year)
    -> std::vector<std::string> {
  std::vector<query_data_detail::SqlParam> params;
  const std::string kSql = query_data_internal::BuildMonthsSql(year, params);

  std::vector<std::string> formatted;
  for (const auto& [current_year, current_month] :
       query_data_detail::QueryYearMonth(db_conn, kSql, params)) {
    formatted.push_back(query_data_detail::FormatDate({.kYear = current_year,
                                                       .kMonth = current_month,
                                                       .day = kFirstDayOfMonth})
                            .substr(0, kYearMonthLength));
  }
  return formatted;
}

auto QueryDays(sqlite3* db_conn, const std::optional<int>& year,
               const std::optional<int>& month,
               const std::optional<std::string>& from_date,
               const std::optional<std::string>& to_date, bool reverse,
               const std::optional<int>& limit) -> std::vector<std::string> {
  std::vector<query_data_detail::SqlParam> params;
  const std::string kSql = query_data_internal::BuildDaysSql(
      year, month, from_date, to_date, reverse, limit, params);
  return query_data_detail::QueryStringColumn(db_conn, kSql, params);
}

auto QueryDatesByFilters(sqlite3* db_conn, const QueryFilters& filters)
    -> std::vector<std::string> {
  std::vector<query_data_detail::SqlParam> params;
  const std::string kSql =
      query_data_internal::BuildDatesByFiltersSql(db_conn, filters, params);
  return query_data_detail::QueryStringColumn(db_conn, kSql, params);
}

auto QueryDayDurations(sqlite3* db_conn, const QueryFilters& filters)
    -> std::vector<DayDurationRow> {
  std::vector<query_data_detail::SqlParam> params;
  const std::string kSql =
      query_data_internal::BuildDayDurationsSql(db_conn, filters, params);
  return query_data_detail::QueryRowsWithTotalDuration(db_conn, kSql, params);
}

auto QueryProjectRootNames(sqlite3* db_conn) -> std::vector<std::string> {
  const std::string kSql =
      query_data_internal::BuildProjectRootNamesSql(db_conn);
  return query_data_detail::QueryStringColumn(db_conn, kSql, {});
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto QueryDayDurationsByRootInDateRange(sqlite3* db_conn,
                                        const std::optional<std::string>& root,
                                        std::string_view from_date,
                                        std::string_view to_date)
    -> std::vector<DayDurationRow> {
  std::vector<query_data_detail::SqlParam> params;
  const query_data_internal::DateRangeBounds kDateRange{
      .from_date = from_date,
      .to_date = to_date,
  };
  const std::string kSql =
      query_data_internal::BuildDayDurationsByRootInDateRangeSql(
          db_conn, root, kDateRange, params);
  return query_data_detail::QueryRowsWithTotalDuration(db_conn, kSql, params);
}

auto QueryActivitySuggestions(sqlite3* db_conn,
                              const ActivitySuggestionQueryOptions& options)
    -> std::vector<ActivitySuggestionRow> {
  const int kLookbackDays = query_data_internal::ClampPositiveOrDefault(
      options.lookback_days, kDefaultLookbackDays);
  const int kLimit = query_data_internal::ClampPositiveOrDefault(
      options.limit, kDefaultSuggestLimit);

  query_data_internal::EnsureProjectPathSnapshotColumnOrThrow(
      db_conn, "QueryActivitySuggestions");
  const std::string kSql =
      query_data_internal::BuildActivitySuggestionsSql(options);
  return query_data_internal::ExecuteActivitySuggestions(db_conn, kSql, options,
                                                         kLookbackDays, kLimit);
}

auto QueryLatestTrackedDate(sqlite3* db_conn) -> std::optional<std::string> {
  const std::string kSql = query_data_internal::BuildLatestTrackedDateSql();
  const auto kDates = query_data_detail::QueryStringColumn(db_conn, kSql, {});
  if (kDates.empty() || kDates.front().empty()) {
    return std::nullopt;
  }
  return kDates.front();
}

auto QueryProjectTree(sqlite3* db_conn, const QueryFilters& filters)
    -> reporting::ProjectTree {
  std::vector<query_data_detail::SqlParam> params;
  const std::string kSql =
      query_data_internal::BuildProjectTreeSql(db_conn, filters, params);
  std::vector<std::pair<std::string, std::int64_t>> records =
      query_data_internal::ExecuteProjectTreeRecords(db_conn, kSql, params);

  reporting::ProjectTree tree;
  query_data_internal::BuildProjectTreeFromRecords(tree, records);
  return tree;
}

}  // namespace tracer::core::infrastructure::query::data
