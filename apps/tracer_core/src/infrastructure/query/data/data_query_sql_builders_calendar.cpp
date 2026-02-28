// infrastructure/query/data/data_query_sql_builders_calendar.cpp
#include <optional>
#include <string>
#include <vector>

#include "infrastructure/query/data/data_query_repository_internal.hpp"
#include "infrastructure/schema/day_schema.hpp"

namespace tracer_core::infrastructure::query::data::internal {
namespace {

constexpr size_t kYearsSqlReserve = 96;
constexpr size_t kMonthsSqlReserve = 112;
constexpr size_t kDaysSqlReserve = 256;
constexpr size_t kLatestTrackedDateSqlReserve = 96;

}  // namespace

auto BuildYearsSql() -> std::string {
  std::string sql;
  sql.reserve(kYearsSqlReserve);
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
  sql.reserve(kMonthsSqlReserve);
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
  sql.reserve(kDaysSqlReserve);
  sql += "SELECT ";
  sql += schema::day::db::kDate;
  sql += " FROM ";
  sql += schema::day::db::kTable;

  const std::vector<std::string> kClauses =
      BuildDayFilterClauses(year, month, from_date, to_date, params);
  AppendWhereClauses(sql, kClauses);

  sql += " ORDER BY ";
  sql += schema::day::db::kDate;
  sql += " ";
  sql += reverse ? "DESC" : "ASC";
  AppendOptionalLimitClause(sql, limit, params);
  sql += ";";

  return sql;
}

auto BuildLatestTrackedDateSql() -> std::string {
  std::string sql;
  sql.reserve(kLatestTrackedDateSqlReserve);
  sql += "SELECT MAX(";
  sql += schema::day::db::kDate;
  sql += ") FROM ";
  sql += schema::day::db::kTable;
  sql += ";";
  return sql;
}

}  // namespace tracer_core::infrastructure::query::data::internal
