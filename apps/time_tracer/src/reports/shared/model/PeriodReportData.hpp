// reports/shared/model/PeriodReportData.hpp
#ifndef PERIOD_REPORT_DATA_HPP
#define PERIOD_REPORT_DATA_HPP

#include <string>
#include <vector>
#include "reports/shared/model/ProjectTree.hpp"

struct PeriodReportData {
    int days_to_query;
    std::string start_date;
    std::string end_date;
    long long total_duration = 0;
    int actual_days = 0;
    
    std::vector<std::pair<std::string, long long>> records;
    
    // [新增] 用于存储 BaseQuerier 聚合查询的结果 (Project ID -> Duration)
    std::vector<std::pair<long long, long long>> project_stats;
    
    reporting::ProjectTree project_tree;
};

#endif // PERIOD_REPORT_DATA_HPP