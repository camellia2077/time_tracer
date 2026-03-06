// infrastructure/query/data/data_query_sql_builders_dates.cpp
#include <sqlite3.h>

#include <optional>
#include <string>
#include <vector>

#include "infrastructure/query/data/data_query_repository_internal.hpp"
#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/sql_aliases.hpp"
#include "infrastructure/schema/sqlite_schema.hpp"

namespace tracer_core::infrastructure::query::data::internal {
namespace {

constexpr size_t kDateDistinctSqlReserve = 80;
constexpr size_t kDayDurationSqlReserve = 240;
constexpr size_t kProjectRootsSqlReserve = 340;
constexpr size_t kRootRangeDurationSqlReserve = 520;
constexpr size_t kProjectTreeSqlReserve = 720;

auto AddRootNotEmptyClauses(std::vector<std::string>& clauses) -> void {
  clauses.emplace_back(
      "tr." + std::string(schema::time_records::db::kProjectPathSnapshot) +
      " IS NOT NULL");
  clauses.emplace_back(
      "tr." + std::string(schema::time_records::db::kProjectPathSnapshot) +
      " <> ''");
}

}  // namespace

auto BuildDatesByFiltersSql(sqlite3* db_conn, const QueryFilters& filters,
                            std::vector<detail::SqlParam>& params)
    -> std::string {
  if (filters.project.has_value() || filters.root.has_value()) {
    EnsureProjectPathSnapshotColumnOrThrow(db_conn, "QueryDatesByFilters");
  }

  std::string sql;
  if (NeedRecordsJoinForFilters(filters)) {
    sql = detail::BuildProjectDateJoinSql();
  } else {
    sql.reserve(kDateDistinctSqlReserve);
    sql += "SELECT DISTINCT d.";
    sql += schema::day::db::kDate;
    sql += " FROM ";
    sql += schema::day::db::kTable;
    sql += " d";
  }

  const std::vector<std::string> kClauses =
      detail::BuildWhereClauses(filters, params);
  AppendWhereClauses(sql, kClauses);

  sql += " ORDER BY d.";
  sql += schema::day::db::kDate;
  sql += " ";
  sql += filters.reverse ? "DESC" : "ASC";
  AppendOptionalLimitClause(sql, filters.limit, params);
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
  sql.reserve(kDayDurationSqlReserve);
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

  const std::vector<std::string> kClauses =
      detail::BuildWhereClauses(filters, params);
  AppendWhereClauses(sql, kClauses);

  sql += " GROUP BY d.";
  sql += schema::day::db::kDate;
  sql += " ORDER BY ";
  sql += schema::sql::alias::kTotalDuration;
  sql += " ";
  sql += filters.reverse ? "DESC" : "ASC";
  AppendOptionalLimitClause(sql, filters.limit, params);
  sql += ";";

  return sql;
}

auto BuildProjectRootNamesSql(sqlite3* db_conn) -> std::string {
  EnsureProjectPathSnapshotColumnOrThrow(db_conn, "QueryProjectRootNames");

  std::string sql;
  sql.reserve(kProjectRootsSqlReserve);
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
  sql.reserve(kRootRangeDurationSqlReserve);
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

auto BuildProjectTreeSql(sqlite3* db_conn, const QueryFilters& filters,
                         std::vector<detail::SqlParam>& params) -> std::string {
  EnsureProjectPathSnapshotColumnOrThrow(db_conn, "QueryProjectTree");

  std::string sql;
  sql.reserve(kProjectTreeSqlReserve);
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
  AddRootNotEmptyClauses(clauses);
  AppendWhereClauses(sql, clauses);

  sql += " GROUP BY tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " ORDER BY tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " ASC;";

  return sql;
}

}  // namespace tracer_core::infrastructure::query::data::internal
