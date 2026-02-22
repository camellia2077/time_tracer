// infrastructure/query/data/data_query_repository_sql.cpp
#include "infrastructure/query/data/data_query_repository_sql.hpp"

#include <sqlite3.h>

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/sqlite_schema.hpp"

namespace time_tracer::infrastructure::query::data::detail {
namespace {

constexpr int kThresholdTwoDigits = 10;
constexpr size_t kProjectDateJoinReserve = 192;

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

[[nodiscard]] auto BuildLikeContains(const std::string& value) -> std::string {
  return "%" + EscapeLikeLiteral(value) + "%";
}

[[nodiscard]] auto BuildRootPrefixLikePattern(const std::string& value)
    -> std::string {
  return EscapeLikeLiteral(value) + "\\_%";
}

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

}  // namespace

auto FormatDate(const DateParts& parts) -> std::string {
  std::string month_str = (parts.kMonth < kThresholdTwoDigits)
                              ? ("0" + std::to_string(parts.kMonth))
                              : std::to_string(parts.kMonth);
  std::string day_str = (parts.day < kThresholdTwoDigits)
                            ? ("0" + std::to_string(parts.day))
                            : std::to_string(parts.day);
  return std::to_string(parts.kYear) + "-" + month_str + "-" + day_str;
}

auto BuildProjectDateJoinSql() -> std::string {
  std::string sql;
  sql.reserve(kProjectDateJoinReserve);
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

auto BuildWhereClauses(const QueryFilters& filters,
                       std::vector<SqlParam>& params)
    -> std::vector<std::string> {
  std::vector<std::string> clauses;
  if (filters.kYear.has_value()) {
    clauses.emplace_back(
        BuildQualifiedClause("d", schema::day::db::kYear, "="));
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.kYear});
  }
  if (filters.kMonth.has_value()) {
    clauses.emplace_back(
        BuildQualifiedClause("d", schema::day::db::kMonth, "="));
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.kMonth});
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
    std::string clause = BuildQualifiedClause(
        "tr", schema::time_records::db::kProjectPathSnapshot, "LIKE");
    clause += " ESCAPE '\\'";
    clauses.push_back(std::move(clause));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = BuildLikeContains(*filters.project)});
  }
  if (filters.root.has_value()) {
    std::string clause = "(tr.";
    clause += schema::time_records::db::kProjectPathSnapshot;
    clause += " = ? OR tr.";
    clause += schema::time_records::db::kProjectPathSnapshot;
    clause += " LIKE ? ESCAPE '\\')";
    clauses.push_back(std::move(clause));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = *filters.root,
                      .int_value = 0});
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = BuildRootPrefixLikePattern(*filters.root),
                      .int_value = 0});
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

auto QueryStringColumn(sqlite3* db_conn, const std::string& sql,
                       const std::vector<SqlParam>& params)
    -> std::vector<std::string> {
  std::vector<std::string> results;
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_conn, sql.c_str(), -1, &stmt, nullptr) !=
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

  int step_result = SQLITE_OK;
  while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW) {
    const unsigned char* text = sqlite3_column_text(stmt, 0);
    if (text != nullptr) {
      results.emplace_back(reinterpret_cast<const char*>(text));
    }
  }
  if (step_result != SQLITE_DONE) {
    const std::string kErrorMessage = sqlite3_errmsg(db_conn);
    sqlite3_finalize(stmt);
    throw std::runtime_error("Failed to execute query: " + kErrorMessage);
  }
  sqlite3_finalize(stmt);
  return results;
}

auto QueryYearMonth(sqlite3* db_conn, const std::string& sql,
                    const std::vector<SqlParam>& params)
    -> std::vector<std::pair<int, int>> {
  std::vector<std::pair<int, int>> results;
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_conn, sql.c_str(), -1, &stmt, nullptr) !=
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

  int step_result = SQLITE_OK;
  while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW) {
    const int kYear = sqlite3_column_int(stmt, 0);
    const int kMonth = sqlite3_column_int(stmt, 1);
    results.emplace_back(kYear, kMonth);
  }
  if (step_result != SQLITE_DONE) {
    const std::string kErrorMessage = sqlite3_errmsg(db_conn);
    sqlite3_finalize(stmt);
    throw std::runtime_error("Failed to execute query: " + kErrorMessage);
  }
  sqlite3_finalize(stmt);
  return results;
}

auto QueryRowsWithTotalDuration(sqlite3* db_conn, const std::string& sql,
                                const std::vector<SqlParam>& params)
    -> std::vector<DayDurationRow> {
  std::vector<DayDurationRow> rows;

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_conn, sql.c_str(), -1, &stmt, nullptr) !=
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

  int step_result = SQLITE_OK;
  while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW) {
    DayDurationRow row;
    const unsigned char* text = sqlite3_column_text(stmt, 0);
    if (text != nullptr) {
      row.date = reinterpret_cast<const char*>(text);
    }
    row.total_seconds = sqlite3_column_int64(stmt, 1);
    rows.push_back(std::move(row));
  }
  if (step_result != SQLITE_DONE) {
    const std::string kErrorMessage = sqlite3_errmsg(db_conn);
    sqlite3_finalize(stmt);
    throw std::runtime_error("Failed to execute query: " + kErrorMessage);
  }

  sqlite3_finalize(stmt);
  return rows;
}

}  // namespace time_tracer::infrastructure::query::data::detail
