// infrastructure/query/data/data_query_repository.cpp
#include "infrastructure/query/data/data_query_repository.hpp"

#include <sqlite3.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "infrastructure/query/data/data_query_repository_sql.hpp"
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

}  // namespace time_tracer::infrastructure::query::data
