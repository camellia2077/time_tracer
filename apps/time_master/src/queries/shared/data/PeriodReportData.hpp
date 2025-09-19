// queries/shared/data/PeriodReportData.hpp
#ifndef PERIOD_REPORT_DATA_HPP
#define PERIOD_REPORT_DATA_HPP

#include <string>
#include <vector>
#include "common/utils/ProjectTree.hpp" // [新增] 引入 ProjectTree

// 用于周期报告的数据
struct PeriodReportData {
    int days_to_query;
    std::string start_date;
    std::string end_date;
    long long total_duration = 0;
    int actual_days = 0;
    std::vector<std::pair<std::string, long long>> records;
    ProjectTree project_tree; // [新增] 项目树现在是数据的一部分
};


#endif // PERIOD_REPORT_DATA_HPP