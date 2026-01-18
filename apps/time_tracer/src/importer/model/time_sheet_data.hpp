// importer/model/time_sheet_data.hpp
#ifndef IMPORTER_MODEL_TIME_SHEET_DATA_HPP_
#define IMPORTER_MODEL_TIME_SHEET_DATA_HPP_

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <optional>
#include "common/model/time_data_models.hpp"

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
    std::string date; // 用于外键关联
};

/**
 * @struct pair_hash
 * @brief A custom hash function for using std::pair in an unordered_set.
 */
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

#endif // IMPORTER_MODEL_TIME_SHEET_DATA_HPP_