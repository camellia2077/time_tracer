// infrastructure/query/data/data_query_repository.cpp
#include "infrastructure/query/data/data_query_repository.hpp"

#include <sqlite3.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "infrastructure/query/data/data_query_repository_internal.hpp"
#include "infrastructure/query/data/data_query_repository_sql.hpp"
#include "infrastructure/reports/data/utils/project_tree_builder.hpp"

namespace time_tracer::infrastructure::query::data {
namespace {

constexpr int kFirstDayOfMonth = 1;
constexpr int kDefaultLookbackDays = 10;
constexpr int kDefaultSuggestLimit = 5;
constexpr int kYearMonthLength = 7;

}  // namespace

auto QueryYears(sqlite3* db_conn) -> std::vector<std::string> {
  const std::string kSql = internal::BuildYearsSql();
  return detail::QueryStringColumn(db_conn, kSql, {});
}

auto QueryMonths(sqlite3* db_conn, const std::optional<int>& year)
    -> std::vector<std::string> {
  std::vector<detail::SqlParam> params;
  const std::string kSql = internal::BuildMonthsSql(year, params);

  std::vector<std::string> formatted;
  for (const auto& [current_year, current_month] :
       detail::QueryYearMonth(db_conn, kSql, params)) {
    formatted.push_back(detail::FormatDate({.kYear = current_year,
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
  std::vector<detail::SqlParam> params;
  const std::string kSql = internal::BuildDaysSql(
      year, month, from_date, to_date, reverse, limit, params);
  return detail::QueryStringColumn(db_conn, kSql, params);
}

auto QueryDatesByFilters(sqlite3* db_conn, const QueryFilters& filters)
    -> std::vector<std::string> {
  std::vector<detail::SqlParam> params;
  const std::string kSql =
      internal::BuildDatesByFiltersSql(db_conn, filters, params);
  return detail::QueryStringColumn(db_conn, kSql, params);
}

auto QueryDayDurations(sqlite3* db_conn, const QueryFilters& filters)
    -> std::vector<DayDurationRow> {
  std::vector<detail::SqlParam> params;
  const std::string kSql =
      internal::BuildDayDurationsSql(db_conn, filters, params);
  return detail::QueryRowsWithTotalDuration(db_conn, kSql, params);
}

auto QueryProjectRootNames(sqlite3* db_conn) -> std::vector<std::string> {
  const std::string kSql = internal::BuildProjectRootNamesSql(db_conn);
  return detail::QueryStringColumn(db_conn, kSql, {});
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto QueryDayDurationsByRootInDateRange(sqlite3* db_conn,
                                        const std::optional<std::string>& root,
                                        std::string_view from_date,
                                        std::string_view to_date)
    -> std::vector<DayDurationRow> {
  std::vector<detail::SqlParam> params;
  const internal::DateRangeBounds kDateRange{
      .from_date = from_date,
      .to_date = to_date,
  };
  const std::string kSql = internal::BuildDayDurationsByRootInDateRangeSql(
      db_conn, root, kDateRange, params);
  return detail::QueryRowsWithTotalDuration(db_conn, kSql, params);
}

auto QueryActivitySuggestions(sqlite3* db_conn,
                              const ActivitySuggestionQueryOptions& options)
    -> std::vector<ActivitySuggestionRow> {
  const int kLookbackDays = internal::ClampPositiveOrDefault(
      options.lookback_days, kDefaultLookbackDays);
  const int kLimit =
      internal::ClampPositiveOrDefault(options.limit, kDefaultSuggestLimit);

  internal::EnsureProjectPathSnapshotColumnOrThrow(db_conn,
                                                   "QueryActivitySuggestions");
  const std::string kSql = internal::BuildActivitySuggestionsSql(options);
  return internal::ExecuteActivitySuggestions(db_conn, kSql, options,
                                              kLookbackDays, kLimit);
}

auto QueryLatestTrackedDate(sqlite3* db_conn) -> std::optional<std::string> {
  const std::string kSql = internal::BuildLatestTrackedDateSql();
  const auto kDates = detail::QueryStringColumn(db_conn, kSql, {});
  if (kDates.empty() || kDates.front().empty()) {
    return std::nullopt;
  }
  return kDates.front();
}

auto QueryProjectTree(sqlite3* db_conn, const QueryFilters& filters)
    -> reporting::ProjectTree {
  std::vector<detail::SqlParam> params;
  const std::string kSql =
      internal::BuildProjectTreeSql(db_conn, filters, params);
  std::vector<std::pair<std::string, long long>> records =
      internal::ExecuteProjectTreeRecords(db_conn, kSql, params);

  reporting::ProjectTree tree;
  BuildProjectTreeFromRecords(tree, records);
  return tree;
}

}  // namespace time_tracer::infrastructure::query::data
