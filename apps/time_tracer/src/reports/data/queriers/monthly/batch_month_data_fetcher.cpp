// reports/data/queriers/monthly/batch_month_data_fetcher.cpp
#include "reports/data/queriers/monthly/batch_month_data_fetcher.hpp"

#include <iostream>
#include <stdexcept>

#include "reports/data/cache/project_name_cache.hpp"
#include "reports/data/queriers/utils/batch_aggregation.hpp"

BatchMonthDataFetcher::BatchMonthDataFetcher(sqlite3* sqlite_db)
    : db_(sqlite_db) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto BatchMonthDataFetcher::FetchAllData()
    -> std::map<std::string, MonthlyReportData> {
  std::map<std::string, MonthlyReportData> all_months_data;

  // [新增] 确保项目名称缓存已加载
  auto& name_cache = ProjectNameCache::Instance();
  name_cache.EnsureLoaded(db_);

  // 1. 获取项目统计 (填充 total_duration)
  std::map<std::string, std::map<long long, long long>> project_agg;
  FetchProjectStats(all_months_data, project_agg);

  // 2. 获取实际天数
  std::map<std::string, int> actual_days;
  FetchActualDays(actual_days);

  // [新增] 3. 为每个月份构建项目树（统一 helper）
  reports::data::batch::FinalizeGroupedAggregationWithDays(
      all_months_data, project_agg, actual_days, name_cache);

  return all_months_data;
}

void BatchMonthDataFetcher::FetchProjectStats(
    std::map<std::string, MonthlyReportData>& all_months_data,
    std::map<std::string, std::map<long long, long long>>& project_agg) {
  sqlite3_stmt* stmt = nullptr;
  const char* sql =
      "SELECT strftime('%Y-%m', date) as ym, project_id, SUM(duration) "
      "FROM time_records "
      "GROUP BY ym, project_id "
      "ORDER BY ym;";

  if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
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
  const char* sql =
      "SELECT strftime('%Y-%m', date) as ym, COUNT(DISTINCT date) "
      "FROM time_records "
      "GROUP BY ym;";

  if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
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
