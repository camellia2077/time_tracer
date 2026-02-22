#include <sqlite3.h>

#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "infrastructure/query/data/data_query_repository_internal.hpp"
#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/sql_aliases.hpp"
#include "infrastructure/schema/sqlite_schema.hpp"

namespace time_tracer::infrastructure::query::data::internal {
namespace {

constexpr size_t kQueryYearsReserve = 96;
constexpr size_t kQueryMonthsReserve = 112;
constexpr size_t kQueryDaysReserve = 256;
constexpr size_t kQueryDayClauseReserve = 24;
constexpr size_t kDateDistinctQueryReserve = 80;
constexpr size_t kDayDurationQueryReserve = 240;
constexpr size_t kProjectRootsQueryReserve = 340;
constexpr size_t kRootRangeDurationQueryReserve = 520;
constexpr size_t kActivitySuggestSnapshotQueryReserve = 1300;
constexpr size_t kProjectTreeSnapshotQueryReserve = 720;
constexpr size_t kLatestDateQueryReserve = 96;

[[nodiscard]] auto EscapeLikeLiteral(const std::string& value) -> std::string {
  std::string escaped;
  escaped.reserve(value.size());
  for (const char kCharacter : value) {
    if (kCharacter == '%' || kCharacter == '_' || kCharacter == '\\') {
      escaped.push_back('\\');
    }
    escaped.push_back(kCharacter);
  }
  return escaped;
}

[[nodiscard]] auto HasProjectPathSnapshotColumn(sqlite3* db_conn) -> bool {
  const std::string kSql =
      std::format("PRAGMA table_info({});", schema::time_records::db::kTable);

  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(db_conn, kSql.c_str(), -1, &statement, nullptr) !=
      SQLITE_OK) {
    return false;
  }

  bool found = false;
  int step_result = SQLITE_OK;
  while ((step_result = sqlite3_step(statement)) == SQLITE_ROW) {
    const unsigned char* name_text = sqlite3_column_text(statement, 1);
    if (name_text == nullptr) {
      continue;
    }
    if (std::string_view(reinterpret_cast<const char*>(name_text)) ==
        schema::time_records::db::kProjectPathSnapshot) {
      found = true;
      break;
    }
  }

  sqlite3_finalize(statement);
  return found;
}

}  // namespace

auto ClampPositiveOrDefault(int value, int fallback) -> int {
  if (value <= 0) {
    return fallback;
  }
  return value;
}

auto BuildRootPrefixLikePattern(std::string_view root) -> std::string {
  return EscapeLikeLiteral(std::string(root)) + "\\_%";
}

auto EnsureProjectPathSnapshotColumnOrThrow(sqlite3* db_conn,
                                            std::string_view query_name)
    -> void {
  if (HasProjectPathSnapshotColumn(db_conn)) {
    return;
  }
  throw std::runtime_error(std::string(query_name) +
                           " requires time_records.project_path_snapshot. "
                           "Please upgrade or rebuild the database.");
}

auto BuildYearsSql() -> std::string {
  std::string sql;
  sql.reserve(kQueryYearsReserve);
  sql += "SELECT DISTINCT ";
  sql += schema::day::db::kYear;
  sql += " FROM ";
  sql += schema::day::db::kTable;
  sql += " ORDER BY ";
  sql += schema::day::db::kYear;
  sql += ";";
  return sql;
}

auto BuildMonthsSql(const std::optional<int>& year,
                    std::vector<detail::SqlParam>& params) -> std::string {
  std::string sql;
  sql.reserve(kQueryMonthsReserve);
  sql += "SELECT DISTINCT ";
  sql += schema::day::db::kYear;
  sql += ", ";
  sql += schema::day::db::kMonth;
  sql += " FROM ";
  sql += schema::day::db::kTable;

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
  return sql;
}

auto BuildDaysSql(const std::optional<int>& year,
                  const std::optional<int>& month,
                  const std::optional<std::string>& from_date,
                  const std::optional<std::string>& to_date, bool reverse,
                  const std::optional<int>& limit,
                  std::vector<detail::SqlParam>& params) -> std::string {
  std::string sql;
  sql.reserve(kQueryDaysReserve);
  sql += "SELECT ";
  sql += schema::day::db::kDate;
  sql += " FROM ";
  sql += schema::day::db::kTable;

  std::vector<std::string> clauses;

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

  return sql;
}

auto BuildDatesByFiltersSql(sqlite3* db_conn, const QueryFilters& filters,
                            std::vector<detail::SqlParam>& params)
    -> std::string {
  const bool kNeedRecordsJoin = filters.project.has_value() ||
                                filters.root.has_value() ||
                                filters.remark.has_value();

  if (filters.project.has_value() || filters.root.has_value()) {
    EnsureProjectPathSnapshotColumnOrThrow(db_conn, "QueryDatesByFilters");
  }

  std::string sql;
  if (kNeedRecordsJoin) {
    sql = detail::BuildProjectDateJoinSql();
  } else {
    sql.reserve(kDateDistinctQueryReserve);
    sql += "SELECT DISTINCT d.";
    sql += schema::day::db::kDate;
    sql += " FROM ";
    sql += schema::day::db::kTable;
    sql += " d";
  }

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

  return sql;
}

auto BuildDayDurationsSql(sqlite3* db_conn, const QueryFilters& filters,
                          std::vector<detail::SqlParam>& params)
    -> std::string {
  if (filters.project.has_value() || filters.root.has_value()) {
    EnsureProjectPathSnapshotColumnOrThrow(db_conn, "QueryDayDurations");
  }

  std::string sql;
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

  return sql;
}

auto BuildProjectRootNamesSql(sqlite3* db_conn) -> std::string {
  EnsureProjectPathSnapshotColumnOrThrow(db_conn, "QueryProjectRootNames");

  std::string sql;
  sql.reserve(kProjectRootsQueryReserve);
  sql += "SELECT DISTINCT CASE";
  sql += " WHEN instr(tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += ", '_') > 0 THEN substr(tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += ", 1, instr(tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += ", '_') - 1)";
  sql += " ELSE tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " END AS root_name";
  sql += " FROM ";
  sql += schema::time_records::db::kTable;
  sql += " tr";
  sql += " WHERE tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " IS NOT NULL AND tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " <> ''";
  sql += " ORDER BY root_name ASC;";

  return sql;
}

auto BuildDayDurationsByRootInDateRangeSql(
    sqlite3* db_conn, const std::optional<std::string>& root,
    const DateRangeBounds& date_range, std::vector<detail::SqlParam>& params)
    -> std::string {
  std::string sql;
  sql.reserve(kRootRangeDurationQueryReserve);
  sql += "SELECT d.";
  sql += schema::day::db::kDate;
  sql += ", COALESCE(SUM(tr.";
  sql += schema::time_records::db::kDuration;
  sql += "), 0) AS ";
  sql += schema::sql::alias::kTotalDuration;
  sql += " FROM ";
  sql += schema::day::db::kTable;
  sql += " d LEFT JOIN ";
  sql += schema::time_records::db::kTable;
  sql += " tr ON tr.";
  sql += schema::time_records::db::kDate;
  sql += " = d.";
  sql += schema::day::db::kDate;

  if (root.has_value() && !root->empty()) {
    EnsureProjectPathSnapshotColumnOrThrow(
        db_conn, "QueryDayDurationsByRootInDateRange");
    sql += " AND (tr.";
    sql += schema::time_records::db::kProjectPathSnapshot;
    sql += " = ? OR tr.";
    sql += schema::time_records::db::kProjectPathSnapshot;
    sql += " LIKE ? ESCAPE '\\')";
    params.push_back({.type = detail::SqlParam::Type::kText,
                      .text_value = *root,
                      .int_value = 0});
    params.push_back({.type = detail::SqlParam::Type::kText,
                      .text_value = BuildRootPrefixLikePattern(*root),
                      .int_value = 0});
  }

  sql += " WHERE d.";
  sql += schema::day::db::kDate;
  sql += " >= ? AND d.";
  sql += schema::day::db::kDate;
  sql += " <= ?";
  params.push_back({.type = detail::SqlParam::Type::kText,
                    .text_value = std::string(date_range.from_date),
                    .int_value = 0});
  params.push_back({.type = detail::SqlParam::Type::kText,
                    .text_value = std::string(date_range.to_date),
                    .int_value = 0});

  sql += " GROUP BY d.";
  sql += schema::day::db::kDate;
  sql += " ORDER BY d.";
  sql += schema::day::db::kDate;
  sql += " ASC;";
  return sql;
}

auto BuildActivitySuggestionsSql(const ActivitySuggestionQueryOptions& options)
    -> std::string {
  std::string sql;
  sql.reserve(kActivitySuggestSnapshotQueryReserve);
  sql += "WITH latest_record AS (";
  sql += "  SELECT MAX(tr.";
  sql += schema::time_records::db::kDate;
  sql += ") AS max_date FROM ";
  sql += schema::time_records::db::kTable;
  sql += " tr";
  sql += "), scored_records AS (";
  sql += "  SELECT tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " AS activity_name, tr.";
  sql += schema::time_records::db::kDuration;
  sql += " AS duration_seconds, tr.";
  sql += schema::time_records::db::kDate;
  sql += " AS record_date, CAST((julianday(lr.max_date) - julianday(tr.";
  sql += schema::time_records::db::kDate;
  sql += ")) AS INTEGER) AS days_ago";
  sql += "    FROM ";
  sql += schema::time_records::db::kTable;
  sql += " tr";
  sql += " CROSS JOIN latest_record lr";
  sql += "   WHERE lr.max_date IS NOT NULL";
  sql += "     AND tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " IS NOT NULL";
  sql += "     AND tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " <> ''";
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

  return sql;
}

auto BuildLatestTrackedDateSql() -> std::string {
  std::string sql;
  sql.reserve(kLatestDateQueryReserve);
  sql += "SELECT MAX(";
  sql += schema::day::db::kDate;
  sql += ") FROM ";
  sql += schema::day::db::kTable;
  sql += ";";
  return sql;
}

auto BuildProjectTreeSql(sqlite3* db_conn, const QueryFilters& filters,
                         std::vector<detail::SqlParam>& params) -> std::string {
  EnsureProjectPathSnapshotColumnOrThrow(db_conn, "QueryProjectTree");

  std::string sql;
  sql.reserve(kProjectTreeSnapshotQueryReserve);
  sql += "SELECT tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
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

  std::vector<std::string> clauses = detail::BuildWhereClauses(filters, params);
  clauses.emplace_back(
      "tr." + std::string(schema::time_records::db::kProjectPathSnapshot) +
      " IS NOT NULL");
  clauses.emplace_back(
      "tr." + std::string(schema::time_records::db::kProjectPathSnapshot) +
      " <> ''");
  if (!clauses.empty()) {
    sql += " WHERE " + clauses[0];
    for (size_t index = 1; index < clauses.size(); ++index) {
      sql += " AND " + clauses[index];
    }
  }
  sql += " GROUP BY tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " ORDER BY tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " ASC;";

  return sql;
}

}  // namespace time_tracer::infrastructure::query::data::internal
