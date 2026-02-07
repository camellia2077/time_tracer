// api/cli/impl/commands/query/data_query_executor.cpp
#include "api/cli/impl/commands/query/data_query_executor.hpp"

#include <sqlite3.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "api/cli/impl/commands/query/query_filters.hpp"
#include "api/cli/impl/utils/date_utils.hpp"
#include "api/cli/impl/utils/sqlite_utils.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"

using time_tracer::cli::impl::commands::query::QueryFilters;
using namespace time_tracer::cli::impl::utils;

namespace {

constexpr int kFirstDayOfMonth = 1;

auto QueryYears(sqlite3* db_conn) -> std::vector<std::string> {
  const std::string kSql = "SELECT DISTINCT year FROM days ORDER BY year;";
  return QueryStringColumn(db_conn, kSql, {});
}

auto QueryMonths(sqlite3* db_conn, const std::optional<int>& year)
    -> std::vector<std::string> {
  std::string sql = "SELECT DISTINCT year, month FROM days";
  std::vector<SqlParam> params;
  if (year.has_value()) {
    sql += " WHERE year = ?";
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *year});
  }
  sql += " ORDER BY year, month;";

  constexpr int kYearMonthLength = 7;
  std::vector<std::string> formatted;
  for (const auto& [current_year, current_month] :
       QueryYearMonth(db_conn, sql, params)) {
    formatted.push_back(
        FormatDate({.year = current_year,
                    .month = current_month,
                    .day = kFirstDayOfMonth})
            .substr(0, kYearMonthLength));  // Use constant for substring length
  }
  return formatted;
}

auto PrintList(const std::vector<std::string>& items) -> void {
  for (const auto& item : items) {
    std::cout << item << "\n";
  }
  std::cout << "Total: " << items.size() << std::endl;
}

auto QueryDays(sqlite3* db_conn, const std::optional<int>& year,
               const std::optional<int>& month,
               const std::optional<std::string>& from_date,
               const std::optional<std::string>& to_date, bool reverse,
               const std::optional<int>& limit) -> std::vector<std::string> {
  std::string sql = "SELECT date FROM days";
  std::vector<std::string> clauses;
  std::vector<SqlParam> params;

  if (year.has_value()) {
    clauses.emplace_back("year = ?");
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *year});
  }
  if (month.has_value()) {
    clauses.emplace_back("month = ?");
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *month});
  }
  if (from_date.has_value()) {
    clauses.emplace_back("date >= ?");
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = *from_date,
                      .int_value = 0});
  }
  if (to_date.has_value()) {
    clauses.emplace_back("date <= ?");
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = *to_date,
                      .int_value = 0});
  }
  if (!clauses.empty()) {
    sql += " WHERE " + clauses[0];
    for (size_t i = 1; i < clauses.size(); ++i) {
      sql += " AND " + clauses[i];
    }
  }
  sql += " ORDER BY date ";
  sql += reverse ? "DESC" : "ASC";
  if (limit.has_value() && *limit > 0) {
    sql += " LIMIT ?";
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *limit});
  }
  sql += ";";
  return QueryStringColumn(db_conn, sql, params);
}

auto BuildProjectCte(bool has_project) -> std::string {
  if (has_project) {
    return "WITH RECURSIVE project_paths(id, path) AS ("
           "  SELECT id, name FROM projects WHERE parent_id IS NULL"
           "  UNION ALL "
           "  SELECT p.id, pp.path || '_' || p.name "
           "  FROM projects p JOIN project_paths pp ON p.parent_id = pp.id"
           ") "
           "SELECT DISTINCT d.date FROM days d "
           "JOIN time_records tr ON tr.date = d.date "
           "JOIN project_paths pp ON tr.project_id = pp.id";
  }
  return "SELECT DISTINCT d.date FROM days d "
         "JOIN time_records tr ON tr.date = d.date";
}

auto BuildWhereClauses(const QueryFilters& filters,
                       std::vector<SqlParam>& params)
    -> std::vector<std::string> {
  std::vector<std::string> clauses;
  if (filters.year.has_value()) {
    clauses.emplace_back("d.year = ?");
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.year});
  }
  if (filters.month.has_value()) {
    clauses.emplace_back("d.month = ?");
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.month});
  }
  if (filters.from_date.has_value()) {
    clauses.emplace_back("d.date >= ?");
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = *filters.from_date,
                      .int_value = 0});
  }
  if (filters.to_date.has_value()) {
    clauses.emplace_back("d.date <= ?");
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = *filters.to_date,
                      .int_value = 0});
  }
  if (filters.day_remark.has_value()) {
    clauses.emplace_back("d.remark LIKE ?");
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = "%" + *filters.day_remark + "%"});
  }
  if (filters.project.has_value()) {
    clauses.emplace_back("pp.path LIKE ? ESCAPE '\\'");
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = BuildLikeContains(*filters.project)});
  }
  if (filters.remark.has_value()) {  // Activity Remark
    clauses.emplace_back("tr.activity_remark LIKE ?");
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = "%" + *filters.remark + "%"});
  }
  if (filters.exercise.has_value()) {
    clauses.emplace_back("d.exercise = ?");
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.exercise});
  }
  if (filters.status.has_value()) {
    clauses.emplace_back("d.status = ?");
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.status});
  }
  if (filters.overnight) {
    clauses.emplace_back(
        "(d.getup_time IS NULL OR d.getup_time = '' OR d.getup_time = "
        "'00:00')");
  }
  return clauses;
}

auto QueryDatesByFilters(sqlite3* db_conn, const QueryFilters& filters)
    -> std::vector<std::string> {
  std::string sql;
  const bool kNeedRecordsJoin =
      filters.project.has_value() || filters.remark.has_value();

  if (kNeedRecordsJoin) {
    sql = BuildProjectCte(filters.project.has_value());
  } else {
    sql = "SELECT DISTINCT d.date FROM days d";
  }

  std::vector<SqlParam> params;
  std::vector<std::string> clauses = BuildWhereClauses(filters, params);

  if (!clauses.empty()) {
    sql += " WHERE " + clauses[0];
    for (size_t i = 1; i < clauses.size(); ++i) {
      sql += " AND " + clauses[i];
    }
  }

  sql += " ORDER BY d.date ";
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

// Helper to parse arguments into struct
auto ParseQueryFilters(const ParsedArgs& args) -> QueryFilters {
  QueryFilters filters;
  if (args.Has("year")) {
    filters.year = args.GetAsInt("year");
  }
  if (args.Has("month")) {
    filters.month = args.GetAsInt("month");
  }
  if (args.Has("from")) {
    filters.from_date = NormalizeDateInput(args.Get("from"), false);
  }
  if (args.Has("to")) {
    filters.to_date = NormalizeDateInput(args.Get("to"), true);
  }
  if (args.Has("remark")) {
    filters.remark = args.Get("remark");
  }
  if (args.Has("day_remark")) {
    filters.day_remark = args.Get("day_remark");
  }
  if (args.Has("project")) {
    filters.project = args.Get("project");
  }
  if (args.Has("exercise")) {
    filters.exercise = args.GetAsInt("exercise");
  }
  if (args.Has("status")) {
    filters.status = args.GetAsInt("status");
  }
  filters.overnight = args.Has("overnight");
  filters.reverse = args.Has("reverse");
  if (args.Has("numbers")) {
    filters.limit = args.GetAsInt("numbers");
  }
  return filters;
}

}  // namespace

DataQueryExecutor::DataQueryExecutor(std::filesystem::path db_path)
    : db_path_(std::move(db_path)) {}

void DataQueryExecutor::Execute(const ParsedArgs& args) {
  // Parse common filters
  QueryFilters filters = ParseQueryFilters(args);

  // Determine Action
  std::string action = "unknown";
  if (args.Has("action")) {
    action = args.Get("action");
  } else if (args.Has("argument")) {
    action = args.Get("argument");
  }

  bool list_years = (action == "years");
  bool list_months = (action == "months");
  bool list_days = (action == "days");
  bool search_mode = (action == "search");

  if (!list_years && !list_months && !list_days && !search_mode) {
    throw std::runtime_error("Invalid query data action: " + action +
                             ". Use years, months, days, or search.");
  }

  DBManager db_manager(db_path_.string());
  if (!db_manager.OpenDatabaseIfNeeded()) {
    throw std::runtime_error("Failed to open database at: " +
                             db_path_.string());
  }
  sqlite3* db_conn = db_manager.GetDbConnection();
  if (db_conn == nullptr) {
    throw std::runtime_error("Database connection is null.");
  }

  if (list_years) {
    PrintList(QueryYears(db_conn));
    return;
  }

  if (list_months) {
    PrintList(QueryMonths(db_conn, filters.year));
    return;
  }

  if (list_days) {
    PrintList(QueryDays(db_conn, filters.year, filters.month, filters.from_date,
                        filters.to_date, filters.reverse, filters.limit));
    return;
  }

  if (search_mode) {
    PrintList(QueryDatesByFilters(db_conn, filters));
    return;
  }
}
