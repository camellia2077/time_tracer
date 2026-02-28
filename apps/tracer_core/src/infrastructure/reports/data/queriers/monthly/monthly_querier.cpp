// infrastructure/reports/data/queriers/monthly/monthly_querier.cpp
#include "infrastructure/reports/data/queriers/monthly/monthly_querier.hpp"

#include <algorithm>
#include <cctype>
#include <format>
#include <stdexcept>

#include "infrastructure/reports/data/cache/project_name_cache.hpp"
#include "infrastructure/reports/data/queriers/utils/batch_aggregation.hpp"
#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/sqlite_schema.hpp"

namespace {
constexpr int kAnaerobicDaysColumnIndex = 5;
}

MonthQuerier::MonthQuerier(sqlite3* sqlite_db, std::string_view year_month)
    : RangeQuerierBase(sqlite_db, year_month) {}

auto MonthQuerier::ValidateInput() const -> bool {
  if (this->param_.length() != static_cast<size_t>(kYearMonthLength)) {
    return false;
  }
  if (this->param_[kDashPosition] != '-') {
    return false;
  }

  return (std::isdigit(this->param_[0]) != 0) &&
         (std::isdigit(this->param_[1]) != 0) &&
         (std::isdigit(this->param_[2]) != 0) &&
         (std::isdigit(this->param_[3]) != 0) &&
         (std::isdigit(this->param_[kMonthStartPosition]) != 0) &&
         (std::isdigit(this->param_[kMonthEndPosition]) != 0);
}

void MonthQuerier::HandleInvalidInput(MonthlyReportData& data) const {
  data.is_valid = false;
}

void MonthQuerier::PrepareData(MonthlyReportData& data) const {
  data.range_label = std::string(param_);
  data.start_date = std::string(param_) + "-01";
  data.end_date = std::string(param_) + "-31";
  data.requested_days = 0;
}

auto MonthQuerier::GetDateConditionSql() const -> std::string {
  return std::format("{} >= ? AND {} <= ?", schema::day::db::kDate,
                     schema::day::db::kDate);
}

void MonthQuerier::BindSqlParameters(sqlite3_stmt* stmt) const {
  std::string start_date = std::string(param_) + "-01";
  std::string end_date = std::string(param_) + "-31";

  sqlite3_bind_text(stmt, 1, start_date.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, end_date.c_str(), -1, SQLITE_TRANSIENT);
}

BatchMonthDataFetcher::BatchMonthDataFetcher(sqlite3* sqlite_db)
    : db_(sqlite_db) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto BatchMonthDataFetcher::FetchAllData()
    -> std::map<std::string, MonthlyReportData> {
  std::map<std::string, MonthlyReportData> all_months_data;

  ProjectNameCache name_cache;
  name_cache.EnsureLoaded(db_);

  std::map<std::string, std::map<long long, long long>> project_agg;
  FetchProjectStats(all_months_data, project_agg);

  std::map<std::string, int> actual_days;
  FetchActualDays(actual_days);

  std::map<std::string, int> status_days;
  std::map<std::string, int> sleep_days;
  std::map<std::string, int> exercise_days;
  std::map<std::string, int> cardio_days;
  std::map<std::string, int> anaerobic_days;
  FetchDayFlagCounts(status_days, sleep_days, exercise_days, cardio_days,
                     anaerobic_days);

  reports::data::batch::FinalizeGroupedAggregationWithDays(
      all_months_data, project_agg, actual_days, name_cache);

  for (auto& [year_month, data] : all_months_data) {
    auto status_it = status_days.find(year_month);
    auto sleep_it = sleep_days.find(year_month);
    auto exercise_it = exercise_days.find(year_month);
    data.status_true_days =
        (status_it != status_days.end()) ? status_it->second : 0;
    data.sleep_true_days =
        (sleep_it != sleep_days.end()) ? sleep_it->second : 0;
    data.exercise_true_days =
        (exercise_it != exercise_days.end()) ? exercise_it->second : 0;
    auto cardio_it = cardio_days.find(year_month);
    auto anaerobic_it = anaerobic_days.find(year_month);
    data.cardio_true_days =
        (cardio_it != cardio_days.end()) ? cardio_it->second : 0;
    data.anaerobic_true_days =
        (anaerobic_it != anaerobic_days.end()) ? anaerobic_it->second : 0;
  }

  return all_months_data;
}

void BatchMonthDataFetcher::FetchProjectStats(
    std::map<std::string, MonthlyReportData>& all_months_data,
    std::map<std::string, std::map<long long, long long>>& project_agg) {
  sqlite3_stmt* stmt = nullptr;
  const std::string kSql = std::format(
      "SELECT strftime('%Y-%m', {0}) as ym, {1}, SUM({2}) "
      "FROM {3} "
      "GROUP BY ym, {1} "
      "ORDER BY ym;",
      schema::time_records::db::kDate, schema::time_records::db::kProjectId,
      schema::time_records::db::kDuration, schema::time_records::db::kTable);

  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for monthly stats.");
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* ym_ptr = sqlite3_column_text(stmt, 0);
    if (ym_ptr == nullptr) {
      continue;
    }

    std::string year_month = reinterpret_cast<const char*>(ym_ptr);
    long long project_id = sqlite3_column_int64(stmt, 1);
    long long duration = sqlite3_column_int64(stmt, 2);

    MonthlyReportData& data = all_months_data[year_month];
    if (data.range_label.empty()) {
      data.range_label = year_month;
      data.start_date = year_month + "-01";
      data.end_date = year_month + "-31";
      data.requested_days = 0;
    }

    project_agg[year_month][project_id] += duration;
    data.total_duration += duration;
  }
  sqlite3_finalize(stmt);
}

void BatchMonthDataFetcher::FetchActualDays(
    std::map<std::string, int>& actual_days) {
  sqlite3_stmt* stmt = nullptr;
  const std::string kSql = std::format(
      "SELECT strftime('%Y-%m', {0}) as ym, COUNT(DISTINCT {0}) "
      "FROM {1} "
      "GROUP BY ym;",
      schema::time_records::db::kDate, schema::time_records::db::kTable);

  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(
        "Failed to prepare statement for monthly actual days.");
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* ym_ptr = sqlite3_column_text(stmt, 0);
    if (ym_ptr == nullptr) {
      continue;
    }

    std::string year_month = reinterpret_cast<const char*>(ym_ptr);
    int days = sqlite3_column_int(stmt, 1);

    actual_days[year_month] = days;
  }
  sqlite3_finalize(stmt);
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void BatchMonthDataFetcher::FetchDayFlagCounts(
    std::map<std::string, int>& status_days,
    std::map<std::string, int>& sleep_days,
    std::map<std::string, int>& exercise_days,
    std::map<std::string, int>& cardio_days,
    std::map<std::string, int>& anaerobic_days) {
  sqlite3_stmt* stmt = nullptr;
  const std::string kSql = std::format(
      "SELECT strftime('%Y-%m', {0}) as ym, "
      "SUM(CASE WHEN {1} != 0 THEN 1 ELSE 0 END), "
      "SUM(CASE WHEN {2} != 0 THEN 1 ELSE 0 END), "
      "SUM(CASE WHEN {3} != 0 THEN 1 ELSE 0 END), "
      "SUM(CASE WHEN {4} > 0 THEN 1 ELSE 0 END), "
      "SUM(CASE WHEN {5} > 0 THEN 1 ELSE 0 END) "
      "FROM {6} "
      "GROUP BY ym;",
      schema::day::db::kDate, schema::day::db::kStatus, schema::day::db::kSleep,
      schema::day::db::kExercise, schema::day::db::kCardioTime,
      schema::day::db::kAnaerobicTime, schema::day::db::kTable);

  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(
        "Failed to prepare statement for monthly flag counts.");
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* ym_ptr = sqlite3_column_text(stmt, 0);
    if (ym_ptr == nullptr) {
      continue;
    }

    std::string year_month = reinterpret_cast<const char*>(ym_ptr);
    status_days[year_month] = sqlite3_column_int(stmt, 1);
    sleep_days[year_month] = sqlite3_column_int(stmt, 2);
    exercise_days[year_month] = sqlite3_column_int(stmt, 3);
    cardio_days[year_month] = sqlite3_column_int(stmt, 4);
    anaerobic_days[year_month] =
        sqlite3_column_int(stmt, kAnaerobicDaysColumnIndex);
  }
  sqlite3_finalize(stmt);
}
// NOLINTEND(bugprone-easily-swappable-parameters)
