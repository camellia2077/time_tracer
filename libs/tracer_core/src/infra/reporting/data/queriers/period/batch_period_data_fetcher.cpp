// infra/reporting/data/queriers/period/batch_period_data_fetcher.cpp
#include "infra/reporting/data/queriers/period/batch_period_data_fetcher.hpp"
#include <sqlite3.h>

#include <algorithm>
#include <cstdint>
#include <format>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include "infra/reporting/data/cache/project_name_cache.hpp"  // 引入名称缓存作为 Provider
#include "infra/reporting/data/queriers/utils/batch_aggregation.hpp"
#include "infra/reporting/data/utils/time_derived_stats.hpp"
#include "infra/reporting/shared/utils/format/time_format.hpp"  // 需要用到 AddDaysToDateStr
#include "infra/schema/day_schema.hpp"
#include "infra/schema/sqlite_schema.hpp"

namespace {
using tracer::core::infrastructure::reports::data::stats::
    IsAnaerobicProjectPath;
using tracer::core::infrastructure::reports::data::stats::IsCardioProjectPath;
using tracer::core::infrastructure::reports::data::stats::IsExerciseProjectPath;
using tracer::core::infrastructure::reports::data::stats::IsStudyProjectPath;
}  // namespace

BatchPeriodDataFetcher::BatchPeriodDataFetcher(
    sqlite3* db_connection,
    const tracer_core::application::ports::IPlatformClock& platform_clock)
    : db_(db_connection), platform_clock_(platform_clock) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
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

  std::string today_str = platform_clock_.TodayLocalDateIso();
  // 计算最大范围的起始日期（包含今天，所以是 max_days - 1）
  std::string max_start_date = AddDaysToDateStr(today_str, -(max_days - 1));

  // 使用本次查询的本地快照，避免跨查询生命周期污染。
  ProjectNameCache name_cache;
  name_cache.EnsureLoaded(db_);

  // 2. 执行一次 SQL 查询，获取最大范围内的所有数据
  std::vector<RawRecord> raw_records;
  struct RawDayFlag {
    std::string date;
    int sleep = 0;
  };
  std::vector<RawDayFlag> raw_day_flags;
  sqlite3_stmt* stmt = nullptr;

  // 查询：日期 >= max_start_date
  const std::string kSql = std::format(
      "SELECT {0}, {1}, {2} FROM {3} WHERE {0} >= ?",
      schema::time_records::db::kDate, schema::time_records::db::kProjectId,
      schema::time_records::db::kDuration, schema::time_records::db::kTable);

  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, max_start_date.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      const unsigned char* date_ptr = sqlite3_column_text(stmt, 0);
      std::string date =
          (date_ptr != nullptr) ? reinterpret_cast<const char*>(date_ptr) : "";
      std::int64_t pid = sqlite3_column_int64(stmt, 1);
      std::int64_t dur = sqlite3_column_int64(stmt, 2);

      raw_records.push_back({std::move(date), pid, dur});
    }
    sqlite3_finalize(stmt);
  } else {
    throw std::runtime_error(
        "Failed to prepare statement for batch period data.");
  }

  sqlite3_stmt* flag_stmt = nullptr;
  const std::string kFlagSql = std::format(
      "SELECT {0}, {1} FROM {2} WHERE {0} >= ?", schema::day::db::kDate,
      schema::day::db::kWakeAnchor, schema::day::db::kTable);
  if (sqlite3_prepare_v2(db_, kFlagSql.c_str(), -1, &flag_stmt, nullptr) ==
      SQLITE_OK) {
    sqlite3_bind_text(flag_stmt, 1, max_start_date.c_str(), -1,
                      SQLITE_TRANSIENT);
    while (sqlite3_step(flag_stmt) == SQLITE_ROW) {
      const unsigned char* date_ptr = sqlite3_column_text(flag_stmt, 0);
      std::string date =
          (date_ptr != nullptr) ? reinterpret_cast<const char*>(date_ptr) : "";
      int sleep = sqlite3_column_int(flag_stmt, 1);
      raw_day_flags.push_back({std::move(date), sleep});
    }
    sqlite3_finalize(flag_stmt);
  } else {
    sqlite3_finalize(flag_stmt);
    throw std::runtime_error(
        "Failed to prepare statement for batch period flags.");
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
    std::map<std::int64_t, std::int64_t> project_agg;
    std::set<std::string> distinct_dates;
    std::set<std::string> status_dates;
    std::set<std::string> exercise_dates;
    std::set<std::string> cardio_dates;
    std::set<std::string> anaerobic_dates;

    for (const auto& record : raw_records) {
      // 字符串日期比较： "2025-01-01" >= "2024-12-31" 依然成立（字典序）
      if (record.date >= data.start_date) {
        project_agg[record.project_id] += record.duration;
        data.total_duration += record.duration;
        distinct_dates.insert(record.date);

        const auto kPathParts = name_cache.GetPathParts(record.project_id);
        std::string project_path;
        if (!kPathParts.empty()) {
          project_path = kPathParts.front();
          for (size_t index = 1; index < kPathParts.size(); ++index) {
            project_path += "_" + kPathParts[index];
          }
        }
        if (IsStudyProjectPath(project_path)) {
          status_dates.insert(record.date);
        }
        if (IsExerciseProjectPath(project_path)) {
          exercise_dates.insert(record.date);
        }
        if (IsCardioProjectPath(project_path)) {
          cardio_dates.insert(record.date);
        }
        if (IsAnaerobicProjectPath(project_path)) {
          anaerobic_dates.insert(record.date);
        }
      }
    }

    for (const auto& flag : raw_day_flags) {
      if (flag.date >= data.start_date) {
        if (flag.sleep != 0) {
          data.wake_anchor_true_days++;
        }
      }
    }

    data.status_true_days = static_cast<int>(status_dates.size());
    data.exercise_true_days = static_cast<int>(exercise_dates.size());
    data.cardio_true_days = static_cast<int>(cardio_dates.size());
    data.anaerobic_true_days = static_cast<int>(anaerobic_dates.size());

    reports::data::batch::FinalizeAggregation(data, project_agg, distinct_dates,
                                              name_cache);
  }

  return results;
}
