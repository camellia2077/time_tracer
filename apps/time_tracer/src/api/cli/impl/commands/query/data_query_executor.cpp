// api/cli/impl/commands/query/data_query_executor.cpp
#include "api/cli/impl/commands/query/data_query_executor.hpp"

#include <sqlite3.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <format>
#include <iomanip>
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
#include "infrastructure/reports/shared/utils/format/time_format.hpp"
#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/sql_aliases.hpp"
#include "infrastructure/schema/time_records_schema.hpp"

using time_tracer::cli::impl::commands::query::QueryFilters;
using namespace time_tracer::cli::impl::utils;

namespace {

constexpr int kFirstDayOfMonth = 1;

struct DayDurationRow {
  std::string date;
  long long total_seconds = 0;
};

struct DayDurationStats {
  int count = 0;
  double mean_seconds = 0.0;
  double variance_seconds = 0.0;
  double stddev_seconds = 0.0;
  double median_seconds = 0.0;
  double p25_seconds = 0.0;
  double p75_seconds = 0.0;
  double p90_seconds = 0.0;
  double p95_seconds = 0.0;
  double min_seconds = 0.0;
  double max_seconds = 0.0;
  double iqr_seconds = 0.0;
  double mad_seconds = 0.0;
};

auto QueryYears(sqlite3* db_conn) -> std::vector<std::string> {
  const std::string kSql = std::format(
      "SELECT DISTINCT {0} FROM {1} ORDER BY {0};",
      schema::day::db::kYear, schema::day::db::kTable);
  return QueryStringColumn(db_conn, kSql, {});
}

auto QueryMonths(sqlite3* db_conn, const std::optional<int>& year)
    -> std::vector<std::string> {
  std::string sql =
      std::format("SELECT DISTINCT {0}, {1} FROM {2}",
                  schema::day::db::kYear, schema::day::db::kMonth,
                  schema::day::db::kTable);
  std::vector<SqlParam> params;
  if (year.has_value()) {
    sql += std::format(" WHERE {} = ?", schema::day::db::kYear);
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *year});
  }
  sql += std::format(" ORDER BY {0}, {1};", schema::day::db::kYear,
                     schema::day::db::kMonth);

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

auto PrintDayDurations(const std::vector<DayDurationRow>& rows) -> void {
  for (const auto& row : rows) {
    std::cout << row.date << " "
              << TimeFormatDuration(row.total_seconds) << "\n";
  }
  std::cout << "Total: " << rows.size() << std::endl;
}

auto FormatDurationSeconds(double seconds) -> std::string {
  if (seconds <= 0.0) {
    return TimeFormatDuration(0);
  }
  return TimeFormatDuration(
      static_cast<long long>(std::llround(seconds)));
}

auto PrintDayDurationStats(const DayDurationStats& stats) -> void {
  constexpr double kSecondsPerHour = 3600.0;
  std::cout << "Days: " << stats.count << "\n";
  if (stats.count == 0) {
    std::cout << "Average: 0h 0m\n";
    std::cout << "Median: 0h 0m\n";
    std::cout << "P25: 0h 0m\n";
    std::cout << "P75: 0h 0m\n";
    std::cout << "P90: 0h 0m\n";
    std::cout << "P95: 0h 0m\n";
    std::cout << "Min: 0h 0m\n";
    std::cout << "Max: 0h 0m\n";
    std::cout << "IQR: 0h 0m\n";
    std::cout << "MAD: 0h 0m\n";
    std::cout << "Variance (h^2): 0.00\n";
    std::cout << "Std Dev: 0.00h (0h 0m)\n";
    std::cout << "\nNotes:\n";
    std::cout << "- Median: middle value after sorting daily durations.\n";
    std::cout << "- P25/P75/P90/P95: nearest-rank percentiles.\n";
    std::cout << "- IQR: P75 - P25, robust spread measure.\n";
    std::cout << "- MAD: median(|x - median|), robust dispersion.\n";
    return;
  }

  std::cout << "Average: " << FormatDurationSeconds(stats.mean_seconds) << "\n";
  std::cout << "Median: " << FormatDurationSeconds(stats.median_seconds)
            << "\n";
  std::cout << "P25: " << FormatDurationSeconds(stats.p25_seconds) << "\n";
  std::cout << "P75: " << FormatDurationSeconds(stats.p75_seconds) << "\n";
  std::cout << "P90: " << FormatDurationSeconds(stats.p90_seconds) << "\n";
  std::cout << "P95: " << FormatDurationSeconds(stats.p95_seconds) << "\n";
  std::cout << "Min: " << FormatDurationSeconds(stats.min_seconds) << "\n";
  std::cout << "Max: " << FormatDurationSeconds(stats.max_seconds) << "\n";
  std::cout << "IQR: " << FormatDurationSeconds(stats.iqr_seconds) << "\n";
  std::cout << "MAD: " << FormatDurationSeconds(stats.mad_seconds) << "\n";

  double variance_hours =
      stats.variance_seconds / (kSecondsPerHour * kSecondsPerHour);
  double std_hours = stats.stddev_seconds / kSecondsPerHour;

  std::cout << std::fixed << std::setprecision(2);
  std::cout << "Variance (h^2): " << variance_hours << "\n";
  std::cout << "Std Dev: " << std_hours << "h ("
            << FormatDurationSeconds(stats.stddev_seconds) << ")\n";

  std::cout << "\nNotes:\n";
  std::cout << "- Median: middle value after sorting daily durations.\n";
  std::cout << "- P25/P75/P90/P95: nearest-rank percentiles.\n";
  std::cout << "- IQR: P75 - P25, robust spread measure.\n";
  std::cout << "- MAD: median(|x - median|), robust dispersion.\n";

  // Reset formatting for any later output.
  std::cout.unsetf(std::ios::floatfield);
  std::cout << std::setprecision(6);
}

auto PrintTopDayDurations(const std::vector<DayDurationRow>& rows,
                          int top_n) -> void {
  if (top_n <= 0 || rows.empty()) {
    return;
  }
  int count = std::min(static_cast<int>(rows.size()), top_n);

  std::cout << "\nTop " << count << " longest days:\n";
  for (int i = 0; i < count; ++i) {
    const auto& row = rows[rows.size() - 1 - i];
    std::cout << "  " << row.date << " "
              << TimeFormatDuration(row.total_seconds) << "\n";
  }

  std::cout << "Top " << count << " shortest days:\n";
  for (int i = 0; i < count; ++i) {
    const auto& row = rows[i];
    std::cout << "  " << row.date << " "
              << TimeFormatDuration(row.total_seconds) << "\n";
  }
}

auto QueryDays(sqlite3* db_conn, const std::optional<int>& year,
               const std::optional<int>& month,
               const std::optional<std::string>& from_date,
               const std::optional<std::string>& to_date, bool reverse,
               const std::optional<int>& limit) -> std::vector<std::string> {
  std::string sql = std::format("SELECT {0} FROM {1}", schema::day::db::kDate,
                                schema::day::db::kTable);
  std::vector<std::string> clauses;
  std::vector<SqlParam> params;

  if (year.has_value()) {
    clauses.emplace_back(std::format("{} = ?", schema::day::db::kYear));
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *year});
  }
  if (month.has_value()) {
    clauses.emplace_back(std::format("{} = ?", schema::day::db::kMonth));
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *month});
  }
  if (from_date.has_value()) {
    clauses.emplace_back(std::format("{} >= ?", schema::day::db::kDate));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = *from_date,
                      .int_value = 0});
  }
  if (to_date.has_value()) {
    clauses.emplace_back(std::format("{} <= ?", schema::day::db::kDate));
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
  sql += std::format(" ORDER BY {} ", schema::day::db::kDate);
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
    return std::format(
        "WITH RECURSIVE {9}({0}, {10}) AS ("
        "  SELECT {0}, {1} FROM {5} p WHERE {2} IS NULL"
        "  UNION ALL "
        "  SELECT p.{0}, pp.{10} || '_' || p.{1} "
        "  FROM {5} p JOIN {9} pp ON p.{2} = pp.{0}"
        ") "
        "SELECT DISTINCT d.{6} FROM {7} d "
        "JOIN {8} tr ON tr.{3} = d.{6} "
        "JOIN {9} pp ON tr.{4} = pp.{0}",
        schema::projects::db::kId, schema::projects::db::kName,
        schema::projects::db::kParentId, schema::time_records::db::kDate,
        schema::time_records::db::kProjectId, schema::projects::db::kTable,
        schema::day::db::kDate, schema::day::db::kTable,
        schema::time_records::db::kTable,
        schema::projects::cte::kProjectPaths, schema::projects::cte::kPath);
  }
  return std::format(
      "SELECT DISTINCT d.{1} FROM {2} d "
      "JOIN {3} tr ON tr.{0} = d.{1}",
      schema::time_records::db::kDate, schema::day::db::kDate,
      schema::day::db::kTable, schema::time_records::db::kTable);
}

auto BuildWhereClauses(const QueryFilters& filters,
                       std::vector<SqlParam>& params)
    -> std::vector<std::string> {
  std::vector<std::string> clauses;
  if (filters.year.has_value()) {
    clauses.emplace_back(std::format("d.{} = ?", schema::day::db::kYear));
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.year});
  }
  if (filters.month.has_value()) {
    clauses.emplace_back(std::format("d.{} = ?", schema::day::db::kMonth));
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.month});
  }
  if (filters.from_date.has_value()) {
    clauses.emplace_back(std::format("d.{} >= ?", schema::day::db::kDate));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = *filters.from_date,
                      .int_value = 0});
  }
  if (filters.to_date.has_value()) {
    clauses.emplace_back(std::format("d.{} <= ?", schema::day::db::kDate));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = *filters.to_date,
                      .int_value = 0});
  }
  if (filters.day_remark.has_value()) {
    clauses.emplace_back(std::format("d.{} LIKE ?", schema::day::db::kRemark));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = "%" + *filters.day_remark + "%"});
  }
  if (filters.project.has_value()) {
    clauses.emplace_back(
        std::format("pp.{} LIKE ? ESCAPE '\\\\'",
                    schema::projects::cte::kPath));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = BuildLikeContains(*filters.project)});
  }
  if (filters.remark.has_value()) {  // Activity Remark
    clauses.emplace_back(
        std::format("tr.{} LIKE ?", schema::time_records::db::kActivityRemark));
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = "%" + *filters.remark + "%"});
  }
  if (filters.exercise.has_value()) {
    clauses.emplace_back(std::format("d.{} = ?", schema::day::db::kExercise));
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.exercise});
  }
  if (filters.status.has_value()) {
    clauses.emplace_back(std::format("d.{} = ?", schema::day::db::kStatus));
    params.push_back({.type = SqlParam::Type::kInt,
                      .text_value = "",
                      .int_value = *filters.status});
  }
  if (filters.overnight) {
    clauses.emplace_back(
        std::format("(d.{0} IS NULL OR d.{0} = '' OR d.{0} = '00:00')",
                    schema::day::db::kGetupTime));
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
    sql = std::format("SELECT DISTINCT d.{0} FROM {1} d",
                      schema::day::db::kDate, schema::day::db::kTable);
  }

  std::vector<SqlParam> params;
  std::vector<std::string> clauses = BuildWhereClauses(filters, params);

  if (!clauses.empty()) {
    sql += " WHERE " + clauses[0];
    for (size_t i = 1; i < clauses.size(); ++i) {
      sql += " AND " + clauses[i];
    }
  }

  sql += std::format(" ORDER BY d.{} ", schema::day::db::kDate);
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
    sql = std::format(
        "WITH RECURSIVE {10}({0}, {11}) AS ("
        "  SELECT {0}, {1} FROM {7} p WHERE {2} IS NULL"
        "  UNION ALL "
        "  SELECT p.{0}, pp.{11} || '_' || p.{1} "
        "  FROM {7} p JOIN {10} pp ON p.{2} = pp.{0}"
        ") "
        "SELECT d.{6}, SUM(tr.{3}) AS {12} "
        "FROM {8} d "
        "JOIN {9} tr ON tr.{4} = d.{6} "
        "JOIN {10} pp ON tr.{5} = pp.{0}",
        schema::projects::db::kId, schema::projects::db::kName,
        schema::projects::db::kParentId, schema::time_records::db::kDuration,
        schema::time_records::db::kDate, schema::time_records::db::kProjectId,
        schema::day::db::kDate, schema::projects::db::kTable,
        schema::day::db::kTable, schema::time_records::db::kTable,
        schema::projects::cte::kProjectPaths, schema::projects::cte::kPath,
        schema::sql::alias::kTotalDuration);
  } else {
    sql = std::format(
        "SELECT d.{2}, SUM(tr.{0}) AS {5} "
        "FROM {3} d "
        "JOIN {4} tr ON tr.{1} = d.{2}",
        schema::time_records::db::kDuration,
        schema::time_records::db::kDate, schema::day::db::kDate,
        schema::day::db::kTable, schema::time_records::db::kTable,
        schema::sql::alias::kTotalDuration);
  }

  std::vector<SqlParam> params;
  std::vector<std::string> clauses = BuildWhereClauses(filters, params);

  if (!clauses.empty()) {
    sql += " WHERE " + clauses[0];
    for (size_t i = 1; i < clauses.size(); ++i) {
      sql += " AND " + clauses[i];
    }
  }

  sql += std::format(" GROUP BY d.{} ", schema::day::db::kDate);
  sql += std::format(" ORDER BY {} ", schema::sql::alias::kTotalDuration);
  sql += filters.reverse ? "DESC" : "ASC";
  if (filters.limit.has_value() && *filters.limit > 0) {
    sql += " LIMIT ?";
    params.push_back(
        {.type = SqlParam::Type::kInt, .text_value = "", .int_value = *filters.limit});
  }
  sql += ";";

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

auto ComputeDayDurationStats(const std::vector<DayDurationRow>& rows)
    -> DayDurationStats {
  DayDurationStats stats;
  if (rows.empty()) {
    return stats;
  }

  std::vector<long long> durations;
  durations.reserve(rows.size());
  for (const auto& row : rows) {
    durations.push_back(row.total_seconds);
  }
  std::sort(durations.begin(), durations.end());

  auto percentile = [&](double p) -> double {
    if (durations.empty()) {
      return 0.0;
    }
    if (p <= 0.0) {
      return static_cast<double>(durations.front());
    }
    if (p >= 100.0) {
      return static_cast<double>(durations.back());
    }
    double rank = std::ceil((p / 100.0) * durations.size());
    size_t index = static_cast<size_t>(
        std::max(1.0, rank)) - 1;
    if (index >= durations.size()) {
      index = durations.size() - 1;
    }
    return static_cast<double>(durations[index]);
  };

  double median = 0.0;
  if (durations.size() % 2 == 1) {
    median = static_cast<double>(durations[durations.size() / 2]);
  } else {
    size_t mid = durations.size() / 2;
    median =
        (static_cast<double>(durations[mid - 1]) +
         static_cast<double>(durations[mid])) /
        2.0;
  }

  std::vector<double> deviations;
  deviations.reserve(durations.size());
  for (const auto& value : durations) {
    deviations.push_back(std::abs(static_cast<double>(value) - median));
  }
  std::sort(deviations.begin(), deviations.end());
  double mad = 0.0;
  if (deviations.size() % 2 == 1) {
    mad = deviations[deviations.size() / 2];
  } else {
    size_t mid = deviations.size() / 2;
    mad = (deviations[mid - 1] + deviations[mid]) / 2.0;
  }

  double mean = 0.0;
  double m2 = 0.0;
  int n = 0;
  for (const auto& value : durations) {
    ++n;
    double x = static_cast<double>(value);
    double delta = x - mean;
    mean += delta / n;
    double delta2 = x - mean;
    m2 += delta * delta2;
  }

  stats.count = n;
  stats.mean_seconds = mean;
  stats.variance_seconds = (n > 0) ? (m2 / n) : 0.0;
  stats.stddev_seconds = std::sqrt(stats.variance_seconds);
  stats.median_seconds = median;
  stats.p25_seconds = percentile(25.0);
  stats.p75_seconds = percentile(75.0);
  stats.p90_seconds = percentile(90.0);
  stats.p95_seconds = percentile(95.0);
  stats.min_seconds = static_cast<double>(durations.front());
  stats.max_seconds = static_cast<double>(durations.back());
  stats.iqr_seconds = stats.p75_seconds - stats.p25_seconds;
  stats.mad_seconds = mad;
  return stats;
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
  bool list_day_durations = (action == "days-duration");
  bool list_day_stats = (action == "days-stats");
  bool search_mode = (action == "search");

  if (!list_years && !list_months && !list_days && !list_day_durations &&
      !list_day_stats && !search_mode) {
    throw std::runtime_error("Invalid query data action: " + action +
                             ". Use years, months, days, days-duration, "
                             "days-stats, or search.");
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

  if (list_day_durations) {
    PrintDayDurations(QueryDayDurations(db_conn, filters));
    return;
  }

  if (list_day_stats) {
    QueryFilters stats_filters = filters;
    stats_filters.limit.reset();
    stats_filters.reverse = false;
    auto rows = QueryDayDurations(db_conn, stats_filters);
    PrintDayDurationStats(ComputeDayDurationStats(rows));
    if (args.Has("top")) {
      int top_n = args.GetAsInt("top");
      PrintTopDayDurations(rows, top_n);
    }
    return;
  }

  if (search_mode) {
    PrintList(QueryDatesByFilters(db_conn, filters));
    return;
  }
}
