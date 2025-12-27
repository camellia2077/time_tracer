// reports/shared/data/DailyReportData.hpp
#ifndef DAILY_REPORT_DATA_HPP
#define DAILY_REPORT_DATA_HPP

#include <string>
#include <vector>
#include <map>
#include <optional>
#include "common/utils/ProjectTree.hpp" 

//用于单个活动的详细记录
struct TimeRecord {
    std::string start_time;
    std::string end_time;
    std::string project_path;
    long long duration_seconds;
    std::optional<std::string> activityRemark;
};

// This struct is part of the data for a daily report, so it belongs here.
struct DayMetadata {
    std::string status = "N/A";
    std::string sleep = "N/A";
    std::string remark = "N/A";
    std::string getup_time = "N/A";
    std::string exercise = "N/A";
};

/**
 * @brief 日查询的结构体
 */
struct DailyReportData {
    std::string date;
    DayMetadata metadata;
    long long total_duration = 0;
    std::vector<std::pair<std::string, long long>> records;
    std::vector<TimeRecord> detailed_records;

    // [核心修改] 使用 Map 存储动态统计数据
    // Key: 数据库列名 (例如 "sleep_total_time", "anaerobic_time")
    // Value: 时长 (秒)
    std::map<std::string, long long> stats; 
    ProjectTree project_tree;
};

#endif // DAILY_REPORT_DATA_HPP