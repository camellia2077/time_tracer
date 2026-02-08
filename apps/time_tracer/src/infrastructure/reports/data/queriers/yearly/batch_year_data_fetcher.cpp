// infrastructure/reports/data/queriers/yearly/batch_year_data_fetcher.cpp
#include "infrastructure/reports/data/queriers/yearly/batch_year_data_fetcher.hpp"

#include <format>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>

#include "infrastructure/reports/data/cache/project_name_cache.hpp"
#include "infrastructure/reports/data/queriers/utils/batch_aggregation.hpp"
#include "infrastructure/reports/shared/utils/format/year_utils.hpp"
#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/time_records_schema.hpp"

BatchYearDataFetcher::BatchYearDataFetcher(sqlite3* sqlite_db)
    : db_(sqlite_db) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto BatchYearDataFetcher::FetchAllData()
    -> std::map<std::string, YearlyReportData> {
  std::map<std::string, YearlyReportData> results;

  auto& name_cache = ProjectNameCache::Instance();
  name_cache.EnsureLoaded(db_);

  sqlite3_stmt* stmt = nullptr;
  const std::string sql = std::format(
      "SELECT strftime('%Y', {0}) as yy, {0}, {1}, SUM({2}) "
      "FROM {3} "
      "GROUP BY yy, {0}, {1} "
      "ORDER BY yy;",
      schema::time_records::db::kDate, schema::time_records::db::kProjectId,
      schema::time_records::db::kDuration, schema::time_records::db::kTable);

  if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for yearly stats.");
  }

  std::map<std::string, std::map<long long, long long>> project_agg;
  std::map<std::string, std::set<std::string>> distinct_dates;

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
  }
  sqlite3_finalize(stmt);

  reports::data::batch::FinalizeGroupedAggregation(results, project_agg,
                                                   distinct_dates, name_cache);

  sqlite3_stmt* flag_stmt = nullptr;
  const std::string flag_sql = std::format(
      "SELECT strftime('%Y', {0}) as yy, "
      "SUM(CASE WHEN {1} != 0 THEN 1 ELSE 0 END), "
      "SUM(CASE WHEN {2} != 0 THEN 1 ELSE 0 END), "
      "SUM(CASE WHEN {3} != 0 THEN 1 ELSE 0 END), "
      "SUM(CASE WHEN {4} > 0 THEN 1 ELSE 0 END), "
      "SUM(CASE WHEN {5} > 0 THEN 1 ELSE 0 END) "
      "FROM {6} "
      "GROUP BY yy;",
      schema::day::db::kDate, schema::day::db::kStatus,
      schema::day::db::kSleep, schema::day::db::kExercise,
      schema::day::db::kCardioTime, schema::day::db::kAnaerobicTime,
      schema::day::db::kTable);
  std::map<std::string, std::tuple<int, int, int, int, int>> flag_counts;

  if (sqlite3_prepare_v2(db_, flag_sql.c_str(), -1, &flag_stmt, nullptr) ==
      SQLITE_OK) {
    while (sqlite3_step(flag_stmt) == SQLITE_ROW) {
      const unsigned char* yy_ptr = sqlite3_column_text(flag_stmt, 0);
      if (yy_ptr == nullptr) {
        continue;
      }
      std::string year_str = reinterpret_cast<const char*>(yy_ptr);
      flag_counts[year_str] = {
          sqlite3_column_int(flag_stmt, 1), sqlite3_column_int(flag_stmt, 2),
          sqlite3_column_int(flag_stmt, 3), sqlite3_column_int(flag_stmt, 4),
          sqlite3_column_int(flag_stmt, 5)};
    }
  } else {
    sqlite3_finalize(flag_stmt);
    throw std::runtime_error("Failed to prepare statement for yearly flags.");
  }
  sqlite3_finalize(flag_stmt);

  for (auto& [year_label, data] : results) {
    auto flag_it = flag_counts.find(year_label);
    if (flag_it == flag_counts.end()) {
      continue;
    }
    data.status_true_days = std::get<0>(flag_it->second);
    data.sleep_true_days = std::get<1>(flag_it->second);
    data.exercise_true_days = std::get<2>(flag_it->second);
    data.cardio_true_days = std::get<3>(flag_it->second);
    data.anaerobic_true_days = std::get<4>(flag_it->second);
  }

  return results;
}
