// reports/data/queriers/weekly/batch_week_data_fetcher.cpp
#include "batch_week_data_fetcher.hpp"

#include <map>
#include <set>
#include <stdexcept>

#include "reports/data/cache/project_name_cache.hpp"
#include "reports/data/utils/project_tree_builder.hpp"
#include "reports/shared/utils/format/iso_week_utils.hpp"

BatchWeekDataFetcher::BatchWeekDataFetcher(sqlite3* sqlite_db)
    : db_(sqlite_db) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto BatchWeekDataFetcher::fetch_all_data()
    -> std::map<std::string, WeeklyReportData> {
  std::map<std::string, WeeklyReportData> results;

  auto& name_cache = ProjectNameCache::instance();
  name_cache.ensure_loaded(db_);

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
    IsoWeek week = iso_week_from_date(date);
    if (week.year <= 0 || week.week <= 0) {
      continue;
    }

    std::string week_label = format_iso_week(week);
    long long project_id = sqlite3_column_int64(stmt, 1);
    long long duration = sqlite3_column_int64(stmt, 2);

    WeeklyReportData& data = results[week_label];
    if (data.range_label.empty()) {
      constexpr int kDaysInWeek = 7;
      data.range_label = week_label;
      data.requested_days = kDaysInWeek;
      data.start_date = iso_week_start_date(week);
      data.end_date = iso_week_end_date(week);
      data.is_valid = true;
    }

    project_agg[week_label][project_id] += duration;
    data.total_duration += duration;
    distinct_dates[week_label].insert(date);
  }
  sqlite3_finalize(stmt);

  for (auto& [week_label, data] : results) {
    data.actual_days = static_cast<int>(distinct_dates[week_label].size());
    const auto& agg = project_agg[week_label];

    data.project_stats.reserve(agg.size());
    for (const auto& [project_id, duration] : agg) {
      data.project_stats.emplace_back(project_id, duration);
    }

    if (data.total_duration > 0) {
      build_project_tree_from_ids(data.project_tree, data.project_stats,
                                  name_cache);
    }
  }

  return results;
}
