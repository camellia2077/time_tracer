// infrastructure/query/data/data_query_repository.cpp
#include "infrastructure/query/data/data_query_repository.hpp"

#include <sqlite3.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "infrastructure/query/data/data_query_repository_sql.hpp"
#include "infrastructure/reports/data/utils/project_tree_builder.hpp"
#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/sql_aliases.hpp"
#include "infrastructure/schema/sqlite_schema.hpp"

namespace time_tracer::infrastructure::query::data {
namespace {

constexpr int kFirstDayOfMonth = 1;
constexpr size_t kQueryYearsReserve = 96;
constexpr size_t kQueryMonthsReserve = 112;
constexpr size_t kQueryDaysReserve = 256;
constexpr size_t kQueryDayClauseReserve = 24;
constexpr size_t kDateDistinctQueryReserve = 80;
constexpr size_t kDayDurationWithProjectQueryReserve = 720;
constexpr size_t kDayDurationQueryReserve = 240;
constexpr size_t kActivitySuggestQueryReserve = 1800;
constexpr size_t kProjectTreeQueryReserve = 2048;
constexpr int kDefaultLookbackDays = 10;
constexpr int kDefaultSuggestLimit = 5;
constexpr int kDurationScoreMode = 1;

[[nodiscard]] auto ClampPositiveOrDefault(int value, int fallback) -> int {
  if (value <= 0) {
    return fallback;
  }
  return value;
}

[[nodiscard]] auto EscapeLikeLiteral(const std::string& value) -> std::string {
  std::string escaped;
  escaped.reserve(value.size());
  for (char character : value) {
    if (character == '%' || character == '_' || character == '\\') {
      escaped.push_back('\\');
    }
    escaped.push_back(character);
  }
  return escaped;
}

}  // namespace

auto QueryYears(sqlite3* db_conn) -> std::vector<std::string> {
  std::string sql;
  sql.reserve(kQueryYearsReserve);
  sql += "SELECT DISTINCT ";
  sql += schema::day::db::kYear;
  sql += " FROM ";
  sql += schema::day::db::kTable;
  sql += " ORDER BY ";
  sql += schema::day::db::kYear;
  sql += ";";
  return detail::QueryStringColumn(db_conn, sql, {});
}

auto QueryMonths(sqlite3* db_conn, const std::optional<int>& year)
    -> std::vector<std::string> {
  std::string sql;
  sql.reserve(kQueryMonthsReserve);
  sql += "SELECT DISTINCT ";
  sql += schema::day::db::kYear;
  sql += ", ";
  sql += schema::day::db::kMonth;
  sql += " FROM ";
  sql += schema::day::db::kTable;

  std::vector<detail::SqlParam> params;
  if (year.has_value()) {
    sql += " WHERE ";
    sql += schema::day::db::kYear;
    sql += " = ?";
    params.push_back({.type = detail::SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *year});
  }
  sql += " ORDER BY ";
  sql += schema::day::db::kYear;
  sql += ", ";
  sql += schema::day::db::kMonth;
  sql += ";";

  constexpr int kYearMonthLength = 7;
  std::vector<std::string> formatted;
  for (const auto& [current_year, current_month] :
       detail::QueryYearMonth(db_conn, sql, params)) {
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
  std::string sql;
  sql.reserve(kQueryDaysReserve);
  sql += "SELECT ";
  sql += schema::day::db::kDate;
  sql += " FROM ";
  sql += schema::day::db::kTable;

  std::vector<std::string> clauses;
  std::vector<detail::SqlParam> params;

  if (year.has_value()) {
    std::string clause;
    clause.reserve(kQueryDayClauseReserve);
    clause += schema::day::db::kYear;
    clause += " = ?";
    clauses.push_back(std::move(clause));
    params.push_back({.type = detail::SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *year});
  }
  if (month.has_value()) {
    std::string clause;
    clause.reserve(kQueryDayClauseReserve);
    clause += schema::day::db::kMonth;
    clause += " = ?";
    clauses.push_back(std::move(clause));
    params.push_back({.type = detail::SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *month});
  }
  if (from_date.has_value()) {
    std::string clause;
    clause.reserve(kQueryDayClauseReserve);
    clause += schema::day::db::kDate;
    clause += " >= ?";
    clauses.push_back(std::move(clause));
    params.push_back({.type = detail::SqlParam::Type::kText,
                      .text_value = *from_date,
                      .int_value = 0});
  }
  if (to_date.has_value()) {
    std::string clause;
    clause.reserve(kQueryDayClauseReserve);
    clause += schema::day::db::kDate;
    clause += " <= ?";
    clauses.push_back(std::move(clause));
    params.push_back({.type = detail::SqlParam::Type::kText,
                      .text_value = *to_date,
                      .int_value = 0});
  }

  if (!clauses.empty()) {
    sql += " WHERE " + clauses[0];
    for (size_t index = 1; index < clauses.size(); ++index) {
      sql += " AND " + clauses[index];
    }
  }

  sql += " ORDER BY ";
  sql += schema::day::db::kDate;
  sql += " ";
  sql += reverse ? "DESC" : "ASC";
  if (limit.has_value() && *limit > 0) {
    sql += " LIMIT ?";
    params.push_back({.type = detail::SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *limit});
  }
  sql += ";";

  return detail::QueryStringColumn(db_conn, sql, params);
}

auto QueryDatesByFilters(sqlite3* db_conn, const QueryFilters& filters)
    -> std::vector<std::string> {
  std::string sql;
  const bool kNeedRecordsJoin =
      filters.project.has_value() || filters.remark.has_value();

  if (kNeedRecordsJoin) {
    sql = detail::BuildProjectCte(filters.project.has_value());
  } else {
    sql.reserve(kDateDistinctQueryReserve);
    sql += "SELECT DISTINCT d.";
    sql += schema::day::db::kDate;
    sql += " FROM ";
    sql += schema::day::db::kTable;
    sql += " d";
  }

  std::vector<detail::SqlParam> params;
  std::vector<std::string> clauses = detail::BuildWhereClauses(filters, params);

  if (!clauses.empty()) {
    sql += " WHERE " + clauses[0];
    for (size_t index = 1; index < clauses.size(); ++index) {
      sql += " AND " + clauses[index];
    }
  }

  sql += " ORDER BY d.";
  sql += schema::day::db::kDate;
  sql += " ";
  sql += filters.reverse ? "DESC" : "ASC";
  if (filters.limit.has_value() && *filters.limit > 0) {
    sql += " LIMIT ?";
    params.push_back({.type = detail::SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.limit});
  }
  sql += ";";

  return detail::QueryStringColumn(db_conn, sql, params);
}

auto QueryDayDurations(sqlite3* db_conn, const QueryFilters& filters)
    -> std::vector<DayDurationRow> {
  std::string sql;
  const bool kNeedProject = filters.project.has_value();

  if (kNeedProject) {
    sql.reserve(kDayDurationWithProjectQueryReserve);
    sql += "WITH RECURSIVE ";
    sql += schema::projects::cte::kProjectPaths;
    sql += "(";
    sql += schema::projects::db::kId;
    sql += ", ";
    sql += schema::projects::cte::kPath;
    sql += ") AS (";
    sql += "  SELECT ";
    sql += schema::projects::db::kId;
    sql += ", ";
    sql += schema::projects::db::kName;
    sql += " FROM ";
    sql += schema::projects::db::kTable;
    sql += " p WHERE ";
    sql += schema::projects::db::kParentId;
    sql += " IS NULL";
    sql += "  UNION ALL ";
    sql += "  SELECT p.";
    sql += schema::projects::db::kId;
    sql += ", pp.";
    sql += schema::projects::cte::kPath;
    sql += " || '_' || p.";
    sql += schema::projects::db::kName;
    sql += "   FROM ";
    sql += schema::projects::db::kTable;
    sql += " p JOIN ";
    sql += schema::projects::cte::kProjectPaths;
    sql += " pp ON p.";
    sql += schema::projects::db::kParentId;
    sql += " = pp.";
    sql += schema::projects::db::kId;
    sql += ") ";
    sql += "SELECT d.";
    sql += schema::day::db::kDate;
    sql += ", SUM(tr.";
    sql += schema::time_records::db::kDuration;
    sql += ") AS ";
    sql += schema::sql::alias::kTotalDuration;
    sql += " FROM ";
    sql += schema::day::db::kTable;
    sql += " d JOIN ";
    sql += schema::time_records::db::kTable;
    sql += " tr ON tr.";
    sql += schema::time_records::db::kDate;
    sql += " = d.";
    sql += schema::day::db::kDate;
    sql += " JOIN ";
    sql += schema::projects::cte::kProjectPaths;
    sql += " pp ON tr.";
    sql += schema::time_records::db::kProjectId;
    sql += " = pp.";
    sql += schema::projects::db::kId;
  } else {
    sql.reserve(kDayDurationQueryReserve);
    sql += "SELECT d.";
    sql += schema::day::db::kDate;
    sql += ", SUM(tr.";
    sql += schema::time_records::db::kDuration;
    sql += ") AS ";
    sql += schema::sql::alias::kTotalDuration;
    sql += " FROM ";
    sql += schema::day::db::kTable;
    sql += " d JOIN ";
    sql += schema::time_records::db::kTable;
    sql += " tr ON tr.";
    sql += schema::time_records::db::kDate;
    sql += " = d.";
    sql += schema::day::db::kDate;
  }

  std::vector<detail::SqlParam> params;
  std::vector<std::string> clauses = detail::BuildWhereClauses(filters, params);

  if (!clauses.empty()) {
    sql += " WHERE " + clauses[0];
    for (size_t index = 1; index < clauses.size(); ++index) {
      sql += " AND " + clauses[index];
    }
  }

  sql += " GROUP BY d.";
  sql += schema::day::db::kDate;
  sql += " ORDER BY ";
  sql += schema::sql::alias::kTotalDuration;
  sql += " ";
  sql += filters.reverse ? "DESC" : "ASC";
  if (filters.limit.has_value() && *filters.limit > 0) {
    sql += " LIMIT ?";
    params.push_back({.type = detail::SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.limit});
  }
  sql += ";";

  return detail::QueryRowsWithTotalDuration(db_conn, sql, params);
}

auto QueryActivitySuggestions(sqlite3* db_conn,
                              const ActivitySuggestionQueryOptions& options)
    -> std::vector<ActivitySuggestionRow> {
  const int lookback_days =
      ClampPositiveOrDefault(options.lookback_days, kDefaultLookbackDays);
  const int limit = ClampPositiveOrDefault(options.limit, kDefaultSuggestLimit);

  std::string sql;
  sql.reserve(kActivitySuggestQueryReserve);
  sql += "WITH RECURSIVE ";
  sql += schema::projects::cte::kProjectPaths;
  sql += "(";
  sql += schema::projects::db::kId;
  sql += ", ";
  sql += schema::projects::cte::kPath;
  sql += ") AS (";
  sql += "  SELECT ";
  sql += schema::projects::db::kId;
  sql += ", ";
  sql += schema::projects::db::kName;
  sql += " FROM ";
  sql += schema::projects::db::kTable;
  sql += " p WHERE ";
  sql += schema::projects::db::kParentId;
  sql += " IS NULL";
  sql += "  UNION ALL ";
  sql += "  SELECT p.";
  sql += schema::projects::db::kId;
  sql += ", pp.";
  sql += schema::projects::cte::kPath;
  sql += " || '_' || p.";
  sql += schema::projects::db::kName;
  sql += "   FROM ";
  sql += schema::projects::db::kTable;
  sql += " p JOIN ";
  sql += schema::projects::cte::kProjectPaths;
  sql += " pp ON p.";
  sql += schema::projects::db::kParentId;
  sql += " = pp.";
  sql += schema::projects::db::kId;
  sql += "), latest_record AS (";
  sql += "  SELECT MAX(tr.";
  sql += schema::time_records::db::kDate;
  sql += ") AS max_date FROM ";
  sql += schema::time_records::db::kTable;
  sql += " tr";
  sql += "), scored_records AS (";
  sql += "  SELECT pp.";
  sql += schema::projects::cte::kPath;
  sql += " AS activity_name, tr.";
  sql += schema::time_records::db::kDuration;
  sql += " AS duration_seconds, tr.";
  sql += schema::time_records::db::kDate;
  sql += " AS record_date, CAST((julianday(lr.max_date) - julianday(tr.";
  sql += schema::time_records::db::kDate;
  sql += ")) AS INTEGER) AS days_ago";
  sql += "    FROM ";
  sql += schema::time_records::db::kTable;
  sql += " tr JOIN ";
  sql += schema::projects::cte::kProjectPaths;
  sql += " pp ON tr.";
  sql += schema::time_records::db::kProjectId;
  sql += " = pp.";
  sql += schema::projects::db::kId;
  sql += " CROSS JOIN latest_record lr";
  sql += "   WHERE lr.max_date IS NOT NULL";
  sql += "), aggregated AS (";
  sql += "  SELECT activity_name, COUNT(*) AS usage_count,";
  sql += "         SUM(duration_seconds) AS total_duration_seconds,";
  sql += "         MAX(record_date) AS last_used_date,";
  sql += "         SUM(CAST(? - days_ago AS REAL)) AS frequency_score,";
  sql +=
      "         SUM(CAST(duration_seconds AS REAL) * CAST(? - days_ago AS "
      "REAL))";
  sql += "             AS duration_score";
  sql += "    FROM scored_records";
  sql += "   WHERE days_ago BETWEEN 0 AND (? - 1)";
  if (options.prefix.has_value() && !options.prefix->empty()) {
    sql += "     AND activity_name LIKE ? ESCAPE '\\\\'";
  }
  sql += "   GROUP BY activity_name";
  sql += ")";
  sql += " SELECT activity_name, usage_count, total_duration_seconds,";
  sql += "        last_used_date,";
  sql +=
      "        CASE WHEN ? = 1 THEN duration_score ELSE frequency_score END AS "
      "score";
  sql += "   FROM aggregated";
  sql += "  ORDER BY score DESC, usage_count DESC, activity_name ASC";
  sql += "  LIMIT ?;";

  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(db_conn, sql.c_str(), -1, &statement, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error("Failed to prepare query.");
  }

  int bind_index = 1;
  sqlite3_bind_int(statement, bind_index++, lookback_days);
  sqlite3_bind_int(statement, bind_index++, lookback_days);
  sqlite3_bind_int(statement, bind_index++, lookback_days);
  if (options.prefix.has_value() && !options.prefix->empty()) {
    std::string like_pattern = EscapeLikeLiteral(*options.prefix) + "%";
    sqlite3_bind_text(statement, bind_index++, like_pattern.c_str(), -1,
                      SQLITE_TRANSIENT);
  }
  sqlite3_bind_int(statement, bind_index++,
                   options.score_by_duration ? kDurationScoreMode : 0);
  sqlite3_bind_int(statement, bind_index, limit);

  std::vector<ActivitySuggestionRow> rows;
  while (sqlite3_step(statement) == SQLITE_ROW) {
    ActivitySuggestionRow row;
    if (const unsigned char* text = sqlite3_column_text(statement, 0);
        text != nullptr) {
      row.activity_name = reinterpret_cast<const char*>(text);
    }
    row.usage_count = sqlite3_column_int(statement, 1);
    row.total_duration_seconds = sqlite3_column_int64(statement, 2);
    if (const unsigned char* text = sqlite3_column_text(statement, 3);
        text != nullptr) {
      row.last_used_date = reinterpret_cast<const char*>(text);
    }
    row.score = sqlite3_column_double(statement, 4);
    rows.push_back(std::move(row));
  }

  sqlite3_finalize(statement);
  return rows;
}

auto QueryLatestTrackedDate(sqlite3* db_conn) -> std::optional<std::string> {
  std::string sql;
  sql.reserve(96);
  sql += "SELECT MAX(";
  sql += schema::day::db::kDate;
  sql += ") FROM ";
  sql += schema::day::db::kTable;
  sql += ";";

  const auto dates = detail::QueryStringColumn(db_conn, sql, {});
  if (dates.empty() || dates.front().empty()) {
    return std::nullopt;
  }
  return dates.front();
}

auto QueryProjectTree(sqlite3* db_conn, const QueryFilters& filters)
    -> reporting::ProjectTree {
  std::string sql;
  sql.reserve(kProjectTreeQueryReserve);
  sql += "WITH RECURSIVE ";
  sql += schema::projects::cte::kProjectPaths;
  sql += "(";
  sql += schema::projects::db::kId;
  sql += ", ";
  sql += schema::projects::cte::kPath;
  sql += ") AS (";
  sql += "  SELECT ";
  sql += schema::projects::db::kId;
  sql += ", ";
  sql += schema::projects::db::kName;
  sql += " FROM ";
  sql += schema::projects::db::kTable;
  sql += " p WHERE ";
  sql += schema::projects::db::kParentId;
  sql += " IS NULL";
  sql += "  UNION ALL ";
  sql += "  SELECT p.";
  sql += schema::projects::db::kId;
  sql += ", pp.";
  sql += schema::projects::cte::kPath;
  sql += " || '_' || p.";
  sql += schema::projects::db::kName;
  sql += "   FROM ";
  sql += schema::projects::db::kTable;
  sql += " p JOIN ";
  sql += schema::projects::cte::kProjectPaths;
  sql += " pp ON p.";
  sql += schema::projects::db::kParentId;
  sql += " = pp.";
  sql += schema::projects::db::kId;
  sql += ") ";
  sql += "SELECT pp.";
  sql += schema::projects::cte::kPath;
  sql += ", SUM(tr.";
  sql += schema::time_records::db::kDuration;
  sql += ") AS ";
  sql += schema::sql::alias::kTotalDuration;
  sql += " FROM ";
  sql += schema::day::db::kTable;
  sql += " d JOIN ";
  sql += schema::time_records::db::kTable;
  sql += " tr ON tr.";
  sql += schema::time_records::db::kDate;
  sql += " = d.";
  sql += schema::day::db::kDate;
  sql += " JOIN ";
  sql += schema::projects::cte::kProjectPaths;
  sql += " pp ON tr.";
  sql += schema::time_records::db::kProjectId;
  sql += " = pp.";
  sql += schema::projects::db::kId;

  std::vector<detail::SqlParam> params;
  std::vector<std::string> clauses = detail::BuildWhereClauses(filters, params);
  if (!clauses.empty()) {
    sql += " WHERE " + clauses[0];
    for (size_t index = 1; index < clauses.size(); ++index) {
      sql += " AND " + clauses[index];
    }
  }
  sql += " GROUP BY pp.";
  sql += schema::projects::cte::kPath;
  sql += " ORDER BY pp.";
  sql += schema::projects::cte::kPath;
  sql += " ASC;";

  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(db_conn, sql.c_str(), -1, &statement, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error("Failed to prepare query.");
  }

  for (size_t index = 0; index < params.size(); ++index) {
    const auto& param = params[index];
    const int bind_index = static_cast<int>(index + 1);
    if (param.type == detail::SqlParam::Type::kInt) {
      sqlite3_bind_int(statement, bind_index, param.int_value);
    } else {
      sqlite3_bind_text(statement, bind_index, param.text_value.c_str(), -1,
                        SQLITE_TRANSIENT);
    }
  }

  std::vector<std::pair<std::string, long long>> records;
  int rc = SQLITE_OK;
  while ((rc = sqlite3_step(statement)) == SQLITE_ROW) {
    const unsigned char* path_text = sqlite3_column_text(statement, 0);
    if (path_text == nullptr) {
      continue;
    }
    std::string path = reinterpret_cast<const char*>(path_text);
    const long long duration_seconds = sqlite3_column_int64(statement, 1);
    records.emplace_back(std::move(path), duration_seconds);
  }
  if (rc != SQLITE_DONE) {
    const std::string error_message = sqlite3_errmsg(db_conn);
    sqlite3_finalize(statement);
    throw std::runtime_error("Failed to execute query: " + error_message);
  }
  sqlite3_finalize(statement);

  reporting::ProjectTree tree;
  BuildProjectTreeFromRecords(tree, records);
  return tree;
}

}  // namespace time_tracer::infrastructure::query::data
