// reports/data/queriers/weekly/batch_week_data_fetcher.cpp
#include "reports/data/queriers/weekly/batch_week_data_fetcher.hpp"

#include <map>
#include <set>
#include <stdexcept>

#include "reports/data/cache/project_name_cache.hpp"
#include "reports/data/queriers/utils/batch_aggregation.hpp"
#include "reports/shared/utils/format/iso_week_utils.hpp"

BatchWeekDataFetcher::BatchWeekDataFetcher(sqlite3* sqlite_db)
    : db_(sqlite_db) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto BatchWeekDataFetcher::FetchAllData()
    -> std::map<std::string, WeeklyReportData> {

  std::map<std::string, WeeklyReportData> results;

  auto& name_cache = ProjectNameCache::Instance();
  name_cache.EnsureLoaded(db_);

  sqlite3_stmt* stmt = nullptr;
  const char* sql =
      "SELECT date, project_id, SUM(duration) "
      "FROM time_records "
      "GROUP BY date, project_id "
      "ORDER BY date;";

  if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for weekly stats.");
  }

  std::map<std::string, std::map<long long, long long>> project_agg;
  std::map<std::string, std::set<std::string>> distinct_dates;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* date_ptr = sqlite3_column_text(stmt, 0);
    if (date_ptr == nullptr) {
      continue;
    }

    std::string date = reinterpret_cast<const char*>(date_ptr);
    IsoWeek week = IsoWeekFromDate(date);
    if (week.year <= 0 || week.week <= 0) {
      continue;
    }

    std::string week_label = FormatIsoWeek(week);
    long long project_id = sqlite3_column_int64(stmt, 1);
    long long duration = sqlite3_column_int64(stmt, 2);

    WeeklyReportData& data = results[week_label];
    if (data.range_label.empty()) {
      constexpr int kDaysInWeek = 7;
      data.range_label = week_label;
      data.requested_days = kDaysInWeek;
      data.start_date = IsoWeekStartDate(week);
      data.end_date = IsoWeekEndDate(week);
      data.is_valid = true;
    }

    project_agg[week_label][project_id] += duration;
    data.total_duration += duration;
    distinct_dates[week_label].insert(date);
  }
  sqlite3_finalize(stmt);

  reports::data::batch::FinalizeGroupedAggregation(results, project_agg,
                                                   distinct_dates, name_cache);

  return results;
}
