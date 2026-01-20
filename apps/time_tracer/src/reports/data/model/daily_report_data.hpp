// reports/data/model/daily_report_data.hpp
#ifndef REPORTS_DATA_MODEL_DAILY_REPORT_DATA_HPP_
#define REPORTS_DATA_MODEL_DAILY_REPORT_DATA_HPP_

#include <string>
#include <vector>
#include <map>
#include <optional>
#include "reports/data/model/project_tree.hpp" 

struct TimeRecord {
    std::string start_time;
    std::string end_time;
    std::string project_path;
    long long duration_seconds;
    std::optional<std::string> activityRemark;
};

struct DayMetadata {
    std::string status = "N/A";
    std::string sleep = "N/A";
    std::string remark = "N/A";
    std::string getup_time = "N/A";
    std::string exercise = "N/A";
};

struct DailyReportData {
    std::string date;
    DayMetadata metadata;
    long long total_duration = 0;
    
    // 旧的字符串记录 (为了兼容性暂时保留)
    std::vector<std::pair<std::string, long long>> records;
    
    // [新增] 聚合后的统计数据 (ID -> Duration)
    // 这是 BaseQuerier 现在填充的数据源
    std::vector<std::pair<long long, long long>> project_stats;

    std::vector<TimeRecord> detailed_records;
    std::map<std::string, long long> stats; 

    // [修改点] 加上命名空间 reporting::
    reporting::ProjectTree project_tree;
};

#endif // REPORTS_DATA_MODEL_DAILY_REPORT_DATA_HPP_