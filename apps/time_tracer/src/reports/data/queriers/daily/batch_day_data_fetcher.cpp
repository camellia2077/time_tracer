// reports/data/queriers/daily/batch_day_data_fetcher.cpp
#include "batch_day_data_fetcher.hpp"

#include <iostream>
#include <stdexcept>

BatchDayDataFetcher::BatchDayDataFetcher(sqlite3* db,
                                         IProjectInfoProvider& provider)
    : db_(db), provider_(provider) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

static auto JoinPathParts(const std::vector<std::string>& parts)
    -> std::string {
  if (parts.empty()) {
    return "";
  }
  std::string path = parts[0];
  for (size_t i = 1; i < parts.size(); ++i) {
    path += "_" + parts[i];
  }
  return path;
}

auto BatchDayDataFetcher::fetch_all_data() -> BatchDataResult {
  BatchDataResult result;

  // 1. [修改] 使用注入的 provider 加载缓存
  provider_.ensure_loaded(db_);

  // 2. 获取天数据
  fetch_days_metadata(result);

  // 3. 获取记录数据并聚合
  fetch_time_records(result);

  return result;
}

// fetch_days_metadata 保持不变 ...
void BatchDayDataFetcher::fetch_days_metadata(BatchDataResult& result) {
  sqlite3_stmt* stmt;
  const char* sql =
      "SELECT date, year, month, "
      "status, sleep, remark, getup_time, exercise, "
      "sleep_total_time, total_exercise_time, anaerobic_time, cardio_time, "
      "grooming_time, "
      "study_time, recreation_time, recreation_zhihu_time, "
      "recreation_bilibili_time, recreation_douyin_time "
      "FROM days ORDER BY date ASC;";

  if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for days metadata.");
  }

  const std::vector<std::string> kStatCols = {"sleep_total_time",
                                              "total_exercise_time",
                                              "anaerobic_time",
                                              "cardio_time",
                                              "grooming_time",
                                              "study_time",
                                              "recreation_time",
                                              "recreation_zhihu_time",
                                              "recreation_bilibili_time",
                                              "recreation_douyin_time"};

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    // ... (保持原逻辑不变) ...
    const char* date_cstr =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    if (date_cstr == nullptr) {
      continue;
    }
    std::string date(date_cstr);
    int year = sqlite3_column_int(stmt, 1);
    int month = sqlite3_column_int(stmt, 2);
    result.date_order.emplace_back(date, year, month);
    DailyReportData& data = result.data_map[date];
    data.date = date;
    data.metadata.status = std::to_string(sqlite3_column_int(stmt, 3));
    data.metadata.sleep = std::to_string(sqlite3_column_int(stmt, 4));
    const unsigned char* rem = sqlite3_column_text(stmt, 5);
    data.metadata.remark =
        (rem != nullptr) ? reinterpret_cast<const char*>(rem) : "N/A";
    const unsigned char* getup = sqlite3_column_text(stmt, 6);
    data.metadata.getup_time =
        (getup != nullptr) ? reinterpret_cast<const char*>(getup) : "N/A";
    data.metadata.exercise = std::to_string(sqlite3_column_int(stmt, 7));
    for (size_t i = 0; i < kStatCols.size(); ++i) {
      data.stats[kStatCols[i]] =
          sqlite3_column_int64(stmt, 8 + static_cast<int>(i));
    }
  }
  sqlite3_finalize(stmt);
}

void BatchDayDataFetcher::fetch_time_records(BatchDataResult& result) {
  sqlite3_stmt* stmt;
  const char* sql =
      "SELECT date, start, end, project_id, duration, activity_remark "
      "FROM time_records ORDER BY date ASC, logical_id ASC;";

  if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for time records.");
  }

  // [修改] 使用注入的 provider
  // auto& cache = ProjectNameCache::instance(); -> 移除
  std::map<std::string, std::map<long long, long long>> temp_aggregation;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const char* date_cstr =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    if (date_cstr == nullptr) {
      continue;
    }
    std::string date(date_cstr);

    auto it = result.data_map.find(date);
    if (it == result.data_map.end()) {
      continue;
    }

    DailyReportData& data = it->second;

    TimeRecord record;
    record.start_time =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    record.end_time =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    long long project_id = sqlite3_column_int64(stmt, 3);
    record.duration_seconds = sqlite3_column_int64(stmt, 4);
    const unsigned char* ar = sqlite3_column_text(stmt, 5);
    if (ar != nullptr) {
      record.activityRemark = reinterpret_cast<const char*>(ar);
    }

    // [修改] 使用 provider_.get_path_parts
    std::vector<std::string> parts = provider_.get_path_parts(project_id);
    record.project_path = JoinPathParts(parts);

    data.detailed_records.push_back(record);
    data.total_duration += record.duration_seconds;

    temp_aggregation[date][project_id] += record.duration_seconds;
  }
  sqlite3_finalize(stmt);

  for (auto& [date, proj_map] : temp_aggregation) {
    auto& data = result.data_map[date];
    data.project_stats.reserve(proj_map.size());
    for (const auto& kv : proj_map) {
      data.project_stats.emplace_back(kv);
    }
  }
}