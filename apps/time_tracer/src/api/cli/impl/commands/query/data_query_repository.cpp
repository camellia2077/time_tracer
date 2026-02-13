// api/cli/impl/commands/query/data_query_repository.cpp
#include "api/cli/impl/commands/query/data_query_repository.hpp"

#include <sqlite3.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "api/cli/impl/utils/date_utils.hpp"
#include "api/cli/impl/utils/sqlite_utils.hpp"
#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/sql_aliases.hpp"
#include "infrastructure/schema/time_records_schema.hpp"

using namespace time_tracer::cli::impl::utils;

namespace time_tracer::cli::impl::commands::query::data {
namespace {

constexpr int kFirstDayOfMonth = 1;
constexpr size_t kProjectCteWithProjectReserve = 640;
constexpr size_t kProjectCteWithoutProjectReserve = 192;
constexpr size_t kQueryYearsReserve = 96;
constexpr size_t kQueryMonthsReserve = 112;
constexpr size_t kQueryDaysReserve = 256;
constexpr size_t kQueryDayClauseReserve = 24;
constexpr size_t kDateDistinctQueryReserve = 80;
constexpr size_t kDayDurationWithProjectQueryReserve = 720;
constexpr size_t kDayDurationQueryReserve = 240;

[[nodiscard]] auto BuildQualifiedName(std::string_view alias,
                                      std::string_view column) -> std::string {
  std::string qualified_name;
  qualified_name.reserve(alias.size() + column.size() + 1);
  qualified_name.append(alias);
  qualified_name.push_back('.');
  qualified_name.append(column);
  return qualified_name;
}

[[nodiscard]] auto BuildQualifiedClause(const char* table_alias,
                                        std::string_view column_name,
                                        const char* comparison_operator)
    -> std::string {
  std::string clause = BuildQualifiedName(table_alias, column_name);
  clause.push_back(' ');
  clause.append(comparison_operator);
  clause.append(" ?");
  return clause;
}

[[nodiscard]] auto BuildProjectCte(bool has_project) -> std::string {
  if (has_project) {
    std::string sql;
    sql.reserve(kProjectCteWithProjectReserve);
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
    sql += "SELECT DISTINCT d.";
    sql += schema::day::db::kDate;
    sql += " FROM ";
    sql += schema::day::db::kTable;
    sql += " d ";
    sql += "JOIN ";
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
    return sql;
  }
  std::string sql;
  sql.reserve(kProjectCteWithoutProjectReserve);
  sql += "SELECT DISTINCT d.";
  sql += schema::day::db::kDate;
  sql += " FROM ";
  sql += schema::day::db::kTable;
  sql += " d JOIN ";
  sql += schema::time_records::db::kTable;
  sql += " tr ON tr.";
  sql += schema::time_records::db::kDate;
  sql += " = d.";
  sql += schema::day::db::kDate;
  return sql;
}

[[nodiscard]] auto BuildWhereClauses(const QueryFilters& filters,
                                     std::vector<SqlParam>& params)
    -> std::vector<std::string> {
  std::vector<std::string> clauses;
  if (filters.year.has_value()) {
    clauses.emplace_back(
        BuildQualifiedClause("d", schema::day::db::kYear, "="));
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.year});
  }
  if (filters.month.has_value()) {
    clauses.emplace_back(
        BuildQualifiedClause("d", schema::day::db::kMonth, "="));
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.month});
  }
  if (filters.from_date.has_value()) {
    clauses.emplace_back(
        BuildQualifiedClause("d", schema::day::db::kDate, ">="));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = *filters.from_date,
                      .int_value = 0});
  }
  if (filters.to_date.has_value()) {
    clauses.emplace_back(
        BuildQualifiedClause("d", schema::day::db::kDate, "<="));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = *filters.to_date,
                      .int_value = 0});
  }
  if (filters.day_remark.has_value()) {
    clauses.emplace_back(
        BuildQualifiedClause("d", schema::day::db::kRemark, "LIKE"));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = "%" + *filters.day_remark + "%"});
  }
  if (filters.project.has_value()) {
    std::string clause =
        BuildQualifiedClause("pp", schema::projects::cte::kPath, "LIKE");
    clause += " ESCAPE '\\\\'";
    clauses.push_back(std::move(clause));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = BuildLikeContains(*filters.project)});
  }
  if (filters.remark.has_value()) {
    clauses.emplace_back(BuildQualifiedClause(
        "tr", schema::time_records::db::kActivityRemark, "LIKE"));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = "%" + *filters.remark + "%"});
  }
  if (filters.exercise.has_value()) {
    clauses.emplace_back(
        BuildQualifiedClause("d", schema::day::db::kExercise, "="));
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.exercise});
  }
  if (filters.status.has_value()) {
    clauses.emplace_back(
        BuildQualifiedClause("d", schema::day::db::kStatus, "="));
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.status});
  }
  if (filters.overnight) {
    std::string clause = "(d.";
    clause += schema::day::db::kGetupTime;
    clause += " IS NULL OR d.";
    clause += schema::day::db::kGetupTime;
    clause += " = '' OR d.";
    clause += schema::day::db::kGetupTime;
    clause += " = '00:00')";
    clauses.push_back(std::move(clause));
  }
  return clauses;
}

[[nodiscard]] auto QueryRowsWithTotalDuration(
    sqlite3* db_conn, const std::string& kSql,
    const std::vector<SqlParam>& params) -> std::vector<DayDurationRow> {
  std::vector<DayDurationRow> rows;

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_conn, kSql.c_str(), -1, &stmt, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error("Failed to prepare query.");
  }

  for (size_t index = 0; index < params.size(); ++index) {
    const auto& param = params[index];
    const int kBindIndex = static_cast<int>(index + 1);
    if (param.type == SqlParam::Type::kInt) {
      sqlite3_bind_int(stmt, kBindIndex, param.int_value);
    } else {
      sqlite3_bind_text(stmt, kBindIndex, param.text_value.c_str(), -1,
                        SQLITE_TRANSIENT);
    }
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    DayDurationRow row;
    const unsigned char* text = sqlite3_column_text(stmt, 0);
    if (text != nullptr) {
      row.date = reinterpret_cast<const char*>(text);
    }
    row.total_seconds = sqlite3_column_int64(stmt, 1);
    rows.push_back(std::move(row));
  }

  sqlite3_finalize(stmt);
  return rows;
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
  return QueryStringColumn(db_conn, sql, {});
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
  std::vector<SqlParam> params;
  if (year.has_value()) {
    sql += " WHERE ";
    sql += schema::day::db::kYear;
    sql += " = ?";
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *year});
  }
  sql += " ORDER BY ";
  sql += schema::day::db::kYear;
  sql += ", ";
  sql += schema::day::db::kMonth;
  sql += ";";

  constexpr int kYearMonthLength = 7;
  std::vector<std::string> formatted;
  for (const auto& [current_year, current_month] :
       QueryYearMonth(db_conn, sql, params)) {
    formatted.push_back(FormatDate({.year = current_year,
                                    .month = current_month,
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
  std::vector<SqlParam> params;

  if (year.has_value()) {
    std::string clause;
    clause.reserve(kQueryDayClauseReserve);
    clause += schema::day::db::kYear;
    clause += " = ?";
    clauses.push_back(std::move(clause));
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *year});
  }
  if (month.has_value()) {
    std::string clause;
    clause.reserve(kQueryDayClauseReserve);
    clause += schema::day::db::kMonth;
    clause += " = ?";
    clauses.push_back(std::move(clause));
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *month});
  }
  if (from_date.has_value()) {
    std::string clause;
    clause.reserve(kQueryDayClauseReserve);
    clause += schema::day::db::kDate;
    clause += " >= ?";
    clauses.push_back(std::move(clause));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = *from_date,
                      .int_value = 0});
  }
  if (to_date.has_value()) {
    std::string clause;
    clause.reserve(kQueryDayClauseReserve);
    clause += schema::day::db::kDate;
    clause += " <= ?";
    clauses.push_back(std::move(clause));
    params.push_back({.type = SqlParam::Type::kText,
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
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *limit});
  }
  sql += ";";

  return QueryStringColumn(db_conn, sql, params);
}

auto QueryDatesByFilters(sqlite3* db_conn, const QueryFilters& filters)
    -> std::vector<std::string> {
  std::string sql;
  const bool kNeedRecordsJoin =
      filters.project.has_value() || filters.remark.has_value();

  if (kNeedRecordsJoin) {
    sql = BuildProjectCte(filters.project.has_value());
  } else {
    sql.reserve(kDateDistinctQueryReserve);
    sql += "SELECT DISTINCT d.";
    sql += schema::day::db::kDate;
    sql += " FROM ";
    sql += schema::day::db::kTable;
    sql += " d";
  }

  std::vector<SqlParam> params;
  std::vector<std::string> clauses = BuildWhereClauses(filters, params);

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
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.limit});
  }
  sql += ";";

  return QueryStringColumn(db_conn, sql, params);
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

  std::vector<SqlParam> params;
  std::vector<std::string> clauses = BuildWhereClauses(filters, params);

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
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.limit});
  }
  sql += ";";

  return QueryRowsWithTotalDuration(db_conn, sql, params);
}

}  // namespace time_tracer::cli::impl::commands::query::data
