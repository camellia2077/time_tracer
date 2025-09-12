// db_inserter/model/time_sheet_model.hpp
#ifndef DATA_STRUCTURES_HPP
#define DATA_STRUCTURES_HPP

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <optional> // 引入 optional

// --- Data Structures ---

/**
 * @struct DayData
 * @brief Holds metadata for a single day.
 */
struct DayData {
    std::string date;
    int year;
    int month;
    int status;
    int sleep;
    std::string remark;
    std::string getup_time;
};

/**
 * @struct TimeRecordInternal
 * @brief Holds data for a single time-logged activity.
 */
struct TimeRecordInternal {
    // --- [核心修改] 新增字段 ---
    long long logical_id;
    long long start_timestamp;
    long long end_timestamp;
    
    std::string date;
    std::string start;
    std::string end;
    std::string project_path;
    int duration_seconds;
    std::optional<std::string> activityRemark; // 新增：用于存储活动的备注
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
        // A simple way to combine hashes
        return h1 ^ (h2 << 1);
    }
};

#endif // DATA_STRUCTURES_HPP