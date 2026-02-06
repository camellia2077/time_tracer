// reports/data/queriers/period/batch_period_data_fetcher.cpp
#include "reports/data/queriers/period/batch_period_data_fetcher.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include "reports/data/cache/project_name_cache.hpp"  // 引入名称缓存作为 Provider
#include "reports/data/queriers/utils/batch_aggregation.hpp"
#include "reports/shared/utils/format/time_format.hpp"  // 需要用到 AddDaysToDateStr

BatchPeriodDataFetcher::BatchPeriodDataFetcher(sqlite3* db_connection)
    : db_(db_connection) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto BatchPeriodDataFetcher::FetchAllData(const std::vector<int>& days_list)
    -> std::map<int, PeriodReportData> {
  std::map<int, PeriodReportData> results;
  if (days_list.empty()) {
    return results;
  }

  // 1. 找出最大天数，确定查询范围
  int max_days = *std::ranges::max_element(days_list);
  if (max_days <= 0) {
    return results;
  }

  std::string today_str = GetCurrentDateStr();
  // 计算最大范围的起始日期（包含今天，所以是 max_days - 1）
  std::string max_start_date = AddDaysToDateStr(today_str, -(max_days - 1));

  // [新增] 确保项目名称缓存已加载 (构建树需要用到)
  auto& name_cache = ProjectNameCache::Instance();
  name_cache.EnsureLoaded(db_);

  // 2. 执行一次 SQL 查询，获取最大范围内的所有数据
  std::vector<RawRecord> raw_records;
  sqlite3_stmt* stmt = nullptr;

  // 查询：日期 >= max_start_date
  const char* sql =
      "SELECT date, project_id, duration FROM time_records WHERE date >= ?";

  if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, max_start_date.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      const unsigned char* date_ptr = sqlite3_column_text(stmt, 0);
      std::string date =
          (date_ptr != nullptr) ? reinterpret_cast<const char*>(date_ptr) : "";
      long long pid = sqlite3_column_int64(stmt, 1);
      long long dur = sqlite3_column_int64(stmt, 2);

      raw_records.push_back({std::move(date), pid, dur});
    }
    sqlite3_finalize(stmt);
  } else {
    throw std::runtime_error(
        "Failed to prepare statement for batch period data.");
  }

  // 3. 在内存中为每个 requested_days 进行聚合
  for (int days : days_list) {
    if (days <= 0) {
      continue;
    }

    PeriodReportData& data = results[days];
    data.requested_days = days;
    data.end_date = today_str;
    data.start_date = AddDaysToDateStr(today_str, -(days - 1));
    data.range_label = std::to_string(days) + " days";

    // 临时聚合容器: Project ID -> Duration
    std::map<long long, long long> project_agg;
    std::set<std::string> distinct_dates;

    for (const auto& record : raw_records) {
      // 字符串日期比较： "2025-01-01" >= "2024-12-31" 依然成立（字典序）
      if (record.date >= data.start_date) {
        project_agg[record.project_id] += record.duration;
        data.total_duration += record.duration;
        distinct_dates.insert(record.date);
      }
    }

    reports::data::batch::FinalizeAggregation(data, project_agg, distinct_dates,
                                              name_cache);
  }

  return results;
}
