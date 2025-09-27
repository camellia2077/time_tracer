// reports/shared/data/DailyReportData.hpp
#ifndef DAILY_REPORT_DATA_HPP
#define DAILY_REPORT_DATA_HPP

#include <string>
#include <vector>
#include <map>
#include <optional>
#include "common/utils/ProjectTree.hpp" // [新增] 引入 ProjectTree

// [新增] 用于单个活动的详细记录
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
    long long sleep_time = 0;
    // --- [核心修改] 新增统计字段 ---
    long long total_exercise_time = 0; // [新增]
    long long anaerobic_time = 0;
    long long cardio_time = 0;
    long long grooming_time = 0;
    // --- [新增字段] ---
    long long recreation_time = 0;
    long long recreation_zhihu_time = 0;
    long long recreation_bilibili_time = 0;
    long long recreation_douyin_time = 0;

    ProjectTree project_tree; // [新增] 项目树现在是数据的一部分
};

#endif // DAILY_REPORT_DATA_HPP