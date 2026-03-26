// infra/reporting/data/queriers/monthly/monthly_querier.cpp
#include "infra/reporting/data/queriers/monthly/monthly_querier.hpp"
#include <sqlite3.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <format>
#include <set>
#include <stdexcept>

#include "infra/reporting/data/cache/project_name_cache.hpp"
#include "infra/reporting/data/queriers/utils/batch_aggregation.hpp"
#include "infra/reporting/data/utils/time_derived_stats.hpp"
#include "infra/schema/day_schema.hpp"
#include "infra/schema/sqlite_schema.hpp"

namespace {
using tracer::core::infrastructure::reports::data::stats::
    IsAnaerobicProjectPath;
using tracer::core::infrastructure::reports::data::stats::IsCardioProjectPath;
using tracer::core::infrastructure::reports::data::stats::IsExerciseProjectPath;
using tracer::core::infrastructure::reports::data::stats::IsStudyProjectPath;

auto JoinPathParts(const std::vector<std::string>& parts) -> std::string {
  if (parts.empty()) {
    return "";
  }
  std::string path = parts[0];
  for (size_t i = 1; i < parts.size(); ++i) {
    path += "_" + parts[i];
  }
  return path;
}
}  // namespace

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

  std::map<std::string, std::map<std::int64_t, std::int64_t>> project_agg;
  std::map<std::string, int> status_days;
  std::map<std::string, int> exercise_days;
  std::map<std::string, int> cardio_days;
  std::map<std::string, int> anaerobic_days;
  FetchProjectStats(all_months_data, project_agg, status_days, exercise_days,
                    cardio_days, anaerobic_days, name_cache);

  std::map<std::string, int> actual_days;
  FetchActualDays(actual_days);

  std::map<std::string, int> sleep_days;
  FetchSleepDays(sleep_days);

  reports::data::batch::FinalizeGroupedAggregationWithDays(
      all_months_data, project_agg, actual_days, name_cache);

  for (auto& [year_month, data] : all_months_data) {
    auto status_it = status_days.find(year_month);
    auto sleep_it = sleep_days.find(year_month);
    auto exercise_it = exercise_days.find(year_month);
    data.status_true_days =
        (status_it != status_days.end()) ? status_it->second : 0;
    data.wake_anchor_true_days =
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
    std::map<std::string, std::map<std::int64_t, std::int64_t>>& project_agg,
    std::map<std::string, int>& status_days,
    std::map<std::string, int>& exercise_days,
    std::map<std::string, int>& cardio_days,
    std::map<std::string, int>& anaerobic_days,
    const IProjectInfoProvider& provider) {
  sqlite3_stmt* stmt = nullptr;
  const std::string kSql = std::format(
      "SELECT strftime('%Y-%m', {0}) as ym, {0}, {1}, SUM({2}) "
      "FROM {3} "
      "GROUP BY ym, {0}, {1} "
      "ORDER BY ym;",
      schema::time_records::db::kDate, schema::time_records::db::kProjectId,
      schema::time_records::db::kDuration, schema::time_records::db::kTable);

  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for monthly stats.");
  }

  std::map<std::string, std::set<std::string>> status_dates;
  std::map<std::string, std::set<std::string>> exercise_dates;
  std::map<std::string, std::set<std::string>> cardio_dates;
  std::map<std::string, std::set<std::string>> anaerobic_dates;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* ym_ptr = sqlite3_column_text(stmt, 0);
    const unsigned char* date_ptr = sqlite3_column_text(stmt, 1);
    if (ym_ptr == nullptr || date_ptr == nullptr) {
      continue;
    }

    std::string year_month = reinterpret_cast<const char*>(ym_ptr);
    std::string date = reinterpret_cast<const char*>(date_ptr);
    std::int64_t project_id = sqlite3_column_int64(stmt, 2);
    std::int64_t duration = sqlite3_column_int64(stmt, 3);

    MonthlyReportData& data = all_months_data[year_month];
    if (data.range_label.empty()) {
      data.range_label = year_month;
      data.start_date = year_month + "-01";
      data.end_date = year_month + "-31";
      data.requested_days = 0;
    }

    project_agg[year_month][project_id] += duration;
    data.total_duration += duration;

    const std::string project_path =
        JoinPathParts(provider.GetPathParts(project_id));
    if (IsStudyProjectPath(project_path)) {
      status_dates[year_month].insert(date);
    }
    if (IsExerciseProjectPath(project_path)) {
      exercise_dates[year_month].insert(date);
    }
    if (IsCardioProjectPath(project_path)) {
      cardio_dates[year_month].insert(date);
    }
    if (IsAnaerobicProjectPath(project_path)) {
      anaerobic_dates[year_month].insert(date);
    }
  }
  sqlite3_finalize(stmt);

  for (const auto& [year_month, dates] : status_dates) {
    status_days[year_month] = static_cast<int>(dates.size());
  }
  for (const auto& [year_month, dates] : exercise_dates) {
    exercise_days[year_month] = static_cast<int>(dates.size());
  }
  for (const auto& [year_month, dates] : cardio_dates) {
    cardio_days[year_month] = static_cast<int>(dates.size());
  }
  for (const auto& [year_month, dates] : anaerobic_dates) {
    anaerobic_days[year_month] = static_cast<int>(dates.size());
  }
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
void BatchMonthDataFetcher::FetchSleepDays(
    std::map<std::string, int>& sleep_days) {
  sqlite3_stmt* stmt = nullptr;
  const std::string kSql = std::format(
      "SELECT strftime('%Y-%m', {0}) as ym, "
      "SUM(CASE WHEN {1} != 0 THEN 1 ELSE 0 END) "
      "FROM {2} "
      "GROUP BY ym;",
      schema::day::db::kDate, schema::day::db::kWakeAnchor,
      schema::day::db::kTable);

  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(
        "Failed to prepare statement for monthly sleep counts.");
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* ym_ptr = sqlite3_column_text(stmt, 0);
    if (ym_ptr == nullptr) {
      continue;
    }

    std::string year_month = reinterpret_cast<const char*>(ym_ptr);
    sleep_days[year_month] = sqlite3_column_int(stmt, 1);
  }
  sqlite3_finalize(stmt);
}
// NOLINTEND(bugprone-easily-swappable-parameters)
