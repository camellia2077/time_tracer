// importer/model/time_sheet_data.hpp
#ifndef IMPORTER_MODEL_TIME_SHEET_DATA_H_
#define IMPORTER_MODEL_TIME_SHEET_DATA_H_

#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "domain/model/time_data_models.hpp"

// --- Data Structures ---

/**
 * @struct DayData
 * @brief Holds metadata for a single day.
 */
struct DayData {
  std::string date;
  std::string remark;
  std::string getup_time;

  int year;
  int month;
  int status;
  int sleep;
  int exercise;

  // [核心修改] 统计数据聚合到 stats 中
  ActivityStats stats;
};

/**
 * @struct TimeRecordInternal
 * @brief Holds data for a single time-logged activity.
 */
// [核心修改] 继承 BaseActivityRecord 复用字段
struct TimeRecordInternal : public BaseActivityRecord {
  // BaseActivityRecord 包含:
  // logical_id, start_timestamp, end_timestamp,
  // start_time_str, end_time_str, project_path,
  // duration_seconds, remark

  // Importer 特有的附加字段
  std::string date;  // 用于外键关联
};

/**
 * @struct PairHash
 * @brief A custom hash function for using std::pair in an unordered_set.
 */
struct PairHash {
  template <class T1, class T2>
  auto operator()(const std::pair<T1, T2>& pair) const -> std::size_t {
    auto hash1 = std::hash<T1>{}(pair.first);
    auto hash2 = std::hash<T2>{}(pair.second);
    return hash1 ^ (hash2 << 1);
  }
};

#endif  // IMPORTER_MODEL_TIME_SHEET_DATA_H_
