// infra/reporting/data/queriers/yearly/yearly_querier.cpp
#include "infra/reporting/data/queriers/yearly/yearly_querier.hpp"
#include <sqlite3.h>

#include <format>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>

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
}  // namespace

YearQuerier::YearQuerier(sqlite3* sqlite_db, std::string_view year_str)
    : RangeQuerierBase(sqlite_db, year_str) {}

auto YearQuerier::ValidateInput() const -> bool {
  int gregorian_year = 0;
  return ParseGregorianYear(param_, gregorian_year);
}

void YearQuerier::HandleInvalidInput(YearlyReportData& data) const {
  data.is_valid = false;
}

void YearQuerier::PrepareData(YearlyReportData& data) const {
  int gregorian_year = 0;
  if (!ParseGregorianYear(param_, gregorian_year)) {
    data.is_valid = false;
    return;
  }

  gregorian_year_ = gregorian_year;
  std::string year_str = FormatGregorianYear(gregorian_year);

  data.range_label = year_str;
  data.requested_days = 0;
  data.start_date = year_str + "-01-01";
  data.end_date = year_str + "-12-31";
  data.is_valid = true;

  start_date_ = data.start_date;
  end_date_ = data.end_date;
}

auto YearQuerier::GetDateConditionSql() const -> std::string {
  return std::format("{} >= ? AND {} <= ?", schema::day::db::kDate,
                     schema::day::db::kDate);
}

void YearQuerier::BindSqlParameters(sqlite3_stmt* stmt) const {
  sqlite3_bind_text(stmt, 1, start_date_.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, end_date_.c_str(), -1, SQLITE_TRANSIENT);
}

BatchYearDataFetcher::BatchYearDataFetcher(sqlite3* sqlite_db)
    : db_(sqlite_db) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto BatchYearDataFetcher::FetchAllData()
    -> std::map<std::string, YearlyReportData> {
  std::map<std::string, YearlyReportData> results;

  ProjectNameCache name_cache;
  name_cache.EnsureLoaded(db_);

  sqlite3_stmt* stmt = nullptr;
  const std::string kSql = std::format(
      "SELECT strftime('%Y', {0}) as yy, {0}, {1}, SUM({2}) "
      "FROM {3} "
      "GROUP BY yy, {0}, {1} "
      "ORDER BY yy;",
      schema::time_records::db::kDate, schema::time_records::db::kProjectId,
      schema::time_records::db::kDuration, schema::time_records::db::kTable);

  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for yearly stats.");
  }

  std::map<std::string, std::map<long long, long long>> project_agg;
  std::map<std::string, std::set<std::string>> distinct_dates;
  std::map<std::string, std::set<std::string>> status_dates;
  std::map<std::string, std::set<std::string>> exercise_dates;
  std::map<std::string, std::set<std::string>> cardio_dates;
  std::map<std::string, std::set<std::string>> anaerobic_dates;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* yy_ptr = sqlite3_column_text(stmt, 0);
    const unsigned char* date_ptr = sqlite3_column_text(stmt, 1);
    if (yy_ptr == nullptr || date_ptr == nullptr) {
      continue;
    }

    std::string year_str = reinterpret_cast<const char*>(yy_ptr);
    std::string date = reinterpret_cast<const char*>(date_ptr);
    int gregorian_year = 0;
    if (!ParseGregorianYear(year_str, gregorian_year)) {
      continue;
    }

    long long project_id = sqlite3_column_int64(stmt, 2);
    long long duration = sqlite3_column_int64(stmt, 3);

    YearlyReportData& data = results[year_str];
    if (data.range_label.empty()) {
      std::string label = FormatGregorianYear(gregorian_year);
      data.range_label = label;
      data.requested_days = 0;
      data.start_date = label + "-01-01";
      data.end_date = label + "-12-31";
      data.is_valid = true;
    }

    project_agg[year_str][project_id] += duration;
    data.total_duration += duration;
    distinct_dates[year_str].insert(date);

    const auto kPathParts = name_cache.GetPathParts(project_id);
    std::string project_path;
    if (!kPathParts.empty()) {
      project_path = kPathParts.front();
      for (size_t index = 1; index < kPathParts.size(); ++index) {
        project_path += "_" + kPathParts[index];
      }
    }
    if (IsStudyProjectPath(project_path)) {
      status_dates[year_str].insert(date);
    }
    if (IsExerciseProjectPath(project_path)) {
      exercise_dates[year_str].insert(date);
    }
    if (IsCardioProjectPath(project_path)) {
      cardio_dates[year_str].insert(date);
    }
    if (IsAnaerobicProjectPath(project_path)) {
      anaerobic_dates[year_str].insert(date);
    }
  }
  sqlite3_finalize(stmt);

  reports::data::batch::FinalizeGroupedAggregation(results, project_agg,
                                                   distinct_dates, name_cache);

  sqlite3_stmt* flag_stmt = nullptr;
  const std::string kFlagSql = std::format(
      "SELECT strftime('%Y', {0}) as yy, "
      "SUM(CASE WHEN {1} != 0 THEN 1 ELSE 0 END) "
      "FROM {2} "
      "GROUP BY yy;",
      schema::day::db::kDate, schema::day::db::kWakeAnchor,
      schema::day::db::kTable);
  std::map<std::string, int> sleep_day_counts;

  if (sqlite3_prepare_v2(db_, kFlagSql.c_str(), -1, &flag_stmt, nullptr) ==
      SQLITE_OK) {
    while (sqlite3_step(flag_stmt) == SQLITE_ROW) {
      const unsigned char* yy_ptr = sqlite3_column_text(flag_stmt, 0);
      if (yy_ptr == nullptr) {
        continue;
      }
      std::string year_str = reinterpret_cast<const char*>(yy_ptr);
      sleep_day_counts[year_str] = sqlite3_column_int(flag_stmt, 1);
    }
  } else {
    sqlite3_finalize(flag_stmt);
    throw std::runtime_error("Failed to prepare statement for yearly flags.");
  }
  sqlite3_finalize(flag_stmt);

  for (auto& [year_label, data] : results) {
    data.status_true_days = static_cast<int>(status_dates[year_label].size());
    const auto sleep_it = sleep_day_counts.find(year_label);
    data.wake_anchor_true_days =
        (sleep_it != sleep_day_counts.end()) ? sleep_it->second : 0;
    data.exercise_true_days =
        static_cast<int>(exercise_dates[year_label].size());
    data.cardio_true_days = static_cast<int>(cardio_dates[year_label].size());
    data.anaerobic_true_days =
        static_cast<int>(anaerobic_dates[year_label].size());
  }

  return results;
}
