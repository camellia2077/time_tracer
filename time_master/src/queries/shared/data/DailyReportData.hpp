// queries/shared/data/DailyReportData.hpp
#ifndef DAILY_REPORT_DATA_HPP
#define DAILY_REPORT_DATA_HPP

#include <string>
#include <vector>
#include <map>

// [新增] 用于单个活动的详细记录
struct TimeRecord {
    std::string start_time;
    std::string end_time;
    std::string project_path;
    long long duration_seconds; // [新增] 存储活动时长
};

// This struct is part of the data for a daily report, so it belongs here.
struct DayMetadata {
    std::string status = "N/A";
    std::string sleep = "N/A"; // sleep 字段 
    std::string remark = "N/A";
    std::string getup_time = "N/A";
};

/**
 * @brief 日查询的结构体
 */
struct DailyReportData {
    std::string date;
    DayMetadata metadata; // The compiler now sees the definition directly above
    long long total_duration = 0;
    std::vector<std::pair<std::string, long long>> records;
    std::vector<TimeRecord> detailed_records; // [新增] 用于存储详细的活动记录
    long long sleep_time = 0; // [新增] 存储睡眠时长，以秒为单位
};

#endif // DAILY_REPORT_DATA_HPP