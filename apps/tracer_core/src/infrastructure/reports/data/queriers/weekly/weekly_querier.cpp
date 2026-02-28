// infrastructure/reports/data/queriers/weekly/weekly_querier.cpp
#include "infrastructure/reports/data/queriers/weekly/weekly_querier.hpp"

#include <format>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>

#include "infrastructure/reports/data/cache/project_name_cache.hpp"
#include "infrastructure/reports/data/queriers/utils/batch_aggregation.hpp"
#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/sqlite_schema.hpp"

WeekQuerier::WeekQuerier(sqlite3* sqlite_db, std::string_view iso_week)
    : RangeQuerierBase(sqlite_db, iso_week) {}

auto WeekQuerier::ValidateInput() const -> bool {
  IsoWeek temp;
  return ParseIsoWeek(param_, temp);
}

void WeekQuerier::HandleInvalidInput(WeeklyReportData& data) const {
  data.is_valid = false;
}

void WeekQuerier::PrepareData(WeeklyReportData& data) const {
  IsoWeek week{};
  if (!ParseIsoWeek(param_, week)) {
    data.is_valid = false;
    return;
  }

  parsed_week_ = week;
  data.range_label = FormatIsoWeek(week);
  constexpr int kDaysInWeek = 7;
  data.requested_days = kDaysInWeek;
  data.start_date = IsoWeekStartDate(week);
  data.end_date = IsoWeekEndDate(week);
  data.is_valid = true;

  start_date_ = data.start_date;
  end_date_ = data.end_date;
}

auto WeekQuerier::GetDateConditionSql() const -> std::string {
  return std::format("{} >= ? AND {} <= ?", schema::day::db::kDate,
                     schema::day::db::kDate);
}

void WeekQuerier::BindSqlParameters(sqlite3_stmt* stmt) const {
  sqlite3_bind_text(stmt, 1, start_date_.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, end_date_.c_str(), -1, SQLITE_TRANSIENT);
}

namespace {
struct DayFlagCounts {
  int status_true_days = 0;
  int sleep_true_days = 0;
  int exercise_true_days = 0;
  int cardio_true_days = 0;
  int anaerobic_true_days = 0;
};

struct WeekRow {
  std::string date;
  IsoWeek week;
  std::string week_label;
};

constexpr int kDaysInWeek = 7;

constexpr int kDateColumn = 0;
constexpr int kProjectIdColumn = 1;
constexpr int kDurationColumn = 2;

constexpr int kFlagStatusColumn = 1;
constexpr int kFlagSleepColumn = 2;
constexpr int kFlagExerciseColumn = 3;
constexpr int kFlagCardioColumn = 4;
constexpr int kFlagAnaerobicColumn = 5;

auto ParseWeekRow(const unsigned char* date_ptr) -> std::optional<WeekRow> {
  if (date_ptr == nullptr) {
    return std::nullopt;
  }
  WeekRow row;
  row.date = reinterpret_cast<const char*>(date_ptr);
  row.week = IsoWeekFromDate(row.date);
  if ((row.week.year <= 0) || (row.week.week <= 0)) {
    return std::nullopt;
  }
  row.week_label = FormatIsoWeek(row.week);
  return row;
}

auto LoadWeeklyFlagCounts(sqlite3* sqlite_db)
    -> std::map<std::string, DayFlagCounts> {
  sqlite3_stmt* flag_stmt = nullptr;
  const std::string kFlagSql = std::format(
      "SELECT {0}, {1}, {2}, {3}, {4}, {5} FROM {6};", schema::day::db::kDate,
      schema::day::db::kStatus, schema::day::db::kSleep,
      schema::day::db::kExercise, schema::day::db::kCardioTime,
      schema::day::db::kAnaerobicTime, schema::day::db::kTable);
  std::map<std::string, DayFlagCounts> flag_counts;

  if (sqlite3_prepare_v2(sqlite_db, kFlagSql.c_str(), -1, &flag_stmt,
                         nullptr) != SQLITE_OK) {
    sqlite3_finalize(flag_stmt);
    throw std::runtime_error("Failed to prepare statement for weekly flags.");
  }

  while (sqlite3_step(flag_stmt) == SQLITE_ROW) {
    auto week_row = ParseWeekRow(sqlite3_column_text(flag_stmt, kDateColumn));
    if (!week_row.has_value()) {
      continue;
    }

    auto& counts = flag_counts[week_row->week_label];
    if (sqlite3_column_int(flag_stmt, kFlagStatusColumn) != 0) {
      counts.status_true_days++;
    }
    if (sqlite3_column_int(flag_stmt, kFlagSleepColumn) != 0) {
      counts.sleep_true_days++;
    }
    if (sqlite3_column_int(flag_stmt, kFlagExerciseColumn) != 0) {
      counts.exercise_true_days++;
    }
    if (sqlite3_column_int(flag_stmt, kFlagCardioColumn) > 0) {
      counts.cardio_true_days++;
    }
    if (sqlite3_column_int(flag_stmt, kFlagAnaerobicColumn) > 0) {
      counts.anaerobic_true_days++;
    }
  }

  sqlite3_finalize(flag_stmt);
  return flag_counts;
}
}  // namespace

BatchWeekDataFetcher::BatchWeekDataFetcher(sqlite3* sqlite_db)
    : db_(sqlite_db) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto BatchWeekDataFetcher::FetchAllData()
    -> std::map<std::string, WeeklyReportData> {
  std::map<std::string, WeeklyReportData> results;

  ProjectNameCache name_cache;
  name_cache.EnsureLoaded(db_);

  sqlite3_stmt* stmt = nullptr;
  const std::string kSql = std::format(
      "SELECT {0}, {1}, SUM({2}) "
      "FROM {3} "
      "GROUP BY {0}, {1} "
      "ORDER BY {0};",
      schema::time_records::db::kDate, schema::time_records::db::kProjectId,
      schema::time_records::db::kDuration, schema::time_records::db::kTable);

  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for weekly stats.");
  }

  std::map<std::string, std::map<long long, long long>> project_agg;
  std::map<std::string, std::set<std::string>> distinct_dates;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    auto week_row = ParseWeekRow(sqlite3_column_text(stmt, kDateColumn));
    if (!week_row.has_value()) {
      continue;
    }

    long long project_id = sqlite3_column_int64(stmt, kProjectIdColumn);
    long long duration = sqlite3_column_int64(stmt, kDurationColumn);

    WeeklyReportData& data = results[week_row->week_label];
    if (data.range_label.empty()) {
      data.range_label = week_row->week_label;
      data.requested_days = kDaysInWeek;
      data.start_date = IsoWeekStartDate(week_row->week);
      data.end_date = IsoWeekEndDate(week_row->week);
      data.is_valid = true;
    }

    project_agg[week_row->week_label][project_id] += duration;
    data.total_duration += duration;
    distinct_dates[week_row->week_label].insert(week_row->date);
  }
  sqlite3_finalize(stmt);

  reports::data::batch::FinalizeGroupedAggregation(results, project_agg,
                                                   distinct_dates, name_cache);

  auto flag_counts = LoadWeeklyFlagCounts(db_);

  for (auto& [week_label, data] : results) {
    auto flag_it = flag_counts.find(week_label);
    if (flag_it == flag_counts.end()) {
      continue;
    }
    data.status_true_days = flag_it->second.status_true_days;
    data.sleep_true_days = flag_it->second.sleep_true_days;
    data.exercise_true_days = flag_it->second.exercise_true_days;
    data.cardio_true_days = flag_it->second.cardio_true_days;
    data.anaerobic_true_days = flag_it->second.anaerobic_true_days;
  }

  return results;
}
