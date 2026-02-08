// infrastructure/reports/data/queriers/daily/batch_day_data_fetcher.cpp
#include "infrastructure/reports/data/queriers/daily/batch_day_data_fetcher.hpp"

#include <format>
#include <iostream>
#include <stdexcept>

#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/time_records_schema.hpp"

BatchDayDataFetcher::BatchDayDataFetcher(sqlite3* sqlite_db,
                                         IProjectInfoProvider& provider)
    : db_(sqlite_db), provider_(provider) {
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

auto BatchDayDataFetcher::FetchAllData() -> BatchDataResult {
  BatchDataResult result;

  // 1. [修改] 使用注入的 provider 加载缓存
  provider_.EnsureLoaded(db_);

  // 2. 获取天数据
  FetchDaysMetadata(result);

  // 2. 获取具体时长数据
  FetchTimeRecords(result);

  return result;
}

// fetch_days_metadata 保持不变 ...
void BatchDayDataFetcher::FetchDaysMetadata(BatchDataResult& result) {
  sqlite3_stmt* stmt;
  const std::string sql = std::format(
      "SELECT {1}, {2}, {3}, "
      "{4}, {5}, {6}, {7}, {8}, "
      "{9}, {10}, {11}, {12}, "
      "{13}, "
      "{14}, {15}, {16}, "
      "{17}, {18} "
      "FROM {0} ORDER BY {1} ASC;",
      schema::day::db::kTable, schema::day::db::kDate,
      schema::day::db::kYear, schema::day::db::kMonth,
      schema::day::db::kStatus, schema::day::db::kSleep,
      schema::day::db::kRemark, schema::day::db::kGetupTime,
      schema::day::db::kExercise, schema::day::db::kSleepTotalTime,
      schema::day::db::kTotalExerciseTime, schema::day::db::kAnaerobicTime,
      schema::day::db::kCardioTime, schema::day::db::kGroomingTime,
      schema::day::db::kStudyTime, schema::day::db::kRecreationTime,
      schema::day::db::kRecreationZhihuTime,
      schema::day::db::kRecreationBilibiliTime,
      schema::day::db::kRecreationDouyinTime);

  if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for days metadata.");
  }

  const std::vector<std::string> kStatCols = {
      std::string(schema::day::db::kSleepTotalTime),
      std::string(schema::day::db::kTotalExerciseTime),
      std::string(schema::day::db::kAnaerobicTime),
      std::string(schema::day::db::kCardioTime),
      std::string(schema::day::db::kGroomingTime),
      std::string(schema::day::db::kStudyTime),
      std::string(schema::day::db::kRecreationTime),
      std::string(schema::day::db::kRecreationZhihuTime),
      std::string(schema::day::db::kRecreationBilibiliTime),
      std::string(schema::day::db::kRecreationDouyinTime)};

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
    constexpr int kColStatus = 3;
    constexpr int kColSleep = 4;
    constexpr int kColRemark = 5;
    constexpr int kColGetup = 6;
    constexpr int kColExercise = 7;
    constexpr int kColStatsStart = 8;

    data.metadata.status = std::to_string(sqlite3_column_int(stmt, kColStatus));
    data.metadata.sleep = std::to_string(sqlite3_column_int(stmt, kColSleep));
    const unsigned char* remark_ptr = sqlite3_column_text(stmt, kColRemark);
    data.metadata.remark = (remark_ptr != nullptr)
                               ? reinterpret_cast<const char*>(remark_ptr)
                               : "N/A";
    const unsigned char* getup_ptr = sqlite3_column_text(stmt, kColGetup);
    data.metadata.getup_time = (getup_ptr != nullptr)
                                   ? reinterpret_cast<const char*>(getup_ptr)
                                   : "N/A";
    data.metadata.exercise =
        std::to_string(sqlite3_column_int(stmt, kColExercise));
    for (size_t i = 0; i < kStatCols.size(); ++i) {
      data.stats[kStatCols[i]] =
          sqlite3_column_int64(stmt, kColStatsStart + static_cast<int>(i));
    }
  }
  sqlite3_finalize(stmt);
}

void BatchDayDataFetcher::FetchTimeRecords(BatchDataResult& result) {
  sqlite3_stmt* stmt;
  const std::string sql = std::format(
      "SELECT {1}, {2}, {3}, {4}, {5}, {6} "
      "FROM {0} ORDER BY {1} ASC, {7} ASC;",
      schema::time_records::db::kTable, schema::time_records::db::kDate,
      schema::time_records::db::kStart, schema::time_records::db::kEnd,
      schema::time_records::db::kProjectId,
      schema::time_records::db::kDuration,
      schema::time_records::db::kActivityRemark,
      schema::time_records::db::kLogicalId);

  if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
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

    auto data_it = result.data_map.find(date);
    if (data_it == result.data_map.end()) {
      continue;
    }

    DailyReportData& data = data_it->second;

    TimeRecord record;
    constexpr int kColStart = 1;
    constexpr int kColEnd = 2;
    constexpr int kColProjectId = 3;
    constexpr int kColDuration = 4;
    constexpr int kColActivityRemark = 5;

    record.start_time =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColStart));
    record.end_time =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColEnd));
    long long project_id = sqlite3_column_int64(stmt, kColProjectId);
    record.duration_seconds = sqlite3_column_int64(stmt, kColDuration);
    const unsigned char* remark_ptr =
        sqlite3_column_text(stmt, kColActivityRemark);
    if (remark_ptr != nullptr) {
      record.activityRemark = reinterpret_cast<const char*>(remark_ptr);
    }

    // [修改] 使用 provider_.get_path_parts
    std::vector<std::string> parts = provider_.GetPathParts(project_id);
    record.project_path = JoinPathParts(parts);

    data.detailed_records.push_back(record);
    data.total_duration += record.duration_seconds;

    temp_aggregation[date][project_id] += record.duration_seconds;
  }
  sqlite3_finalize(stmt);

  for (auto& [date, proj_map] : temp_aggregation) {
    auto& data = result.data_map[date];
    data.project_stats.reserve(proj_map.size());
    for (const auto& project_entry : proj_map) {
      data.project_stats.emplace_back(project_entry);
    }
  }
}
