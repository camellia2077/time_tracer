// reports/shared/data/MonthlyReportData.hpp
#ifndef MONTHLY_REPORT_DATA_HPP
#define MONTHLY_REPORT_DATA_HPP

#include <string>
#include <vector>
#include "reports/shared/model/ProjectTree.hpp"

struct MonthlyReportData {
    std::string year_month;
    long long total_duration = 0;
    int actual_days = 0;
    
    std::vector<std::pair<std::string, long long>> records;
    
    // [新增] 用于存储 BaseQuerier 聚合查询的结果 (Project ID -> Duration)
    std::vector<std::pair<long long, long long>> project_stats;

    reporting::ProjectTree project_tree;
};

#endif // MONTHLY_REPORT_DATA_HPP