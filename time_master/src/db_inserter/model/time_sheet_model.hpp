#ifndef DATA_STRUCTURES_HPP
#define DATA_STRUCTURES_HPP

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>

// db_inserter/model/time_sheet_model.h

// --- Data Structures ---

/**
 * @struct DayData
 * @brief Holds metadata for a single day.
 */
struct DayData {
    std::string date;
    int year;
    int month;
    int status; // [修改] 使用 int 来存储布尔值
    int sleep;  // [修改] 使用 int 来存储布尔值
    std::string remark;
    std::string getup_time;
};

/**
 * @struct TimeRecordInternal
 * @brief Holds data for a single time-logged activity.
 */
struct TimeRecordInternal {
    std::string date;
    std::string start;
    std::string end;
    std::string project_path;
    int duration_seconds;
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