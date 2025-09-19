// queries/shared/data/MonthlyReportData.hpp
#ifndef REPORT_DATA_HPP
#define REPORT_DATA_HPP

#include <string>
#include <vector>
#include "common/utils/ProjectTree.hpp" // [新增] 引入 ProjectTree

// 用于月报的数据
struct MonthlyReportData {
    std::string year_month;
    long long total_duration = 0;
    int actual_days = 0;
    std::vector<std::pair<std::string, long long>> records;
    ProjectTree project_tree; // [新增] 项目树现在是数据的一部分
};

#endif // REPORT_DATA_HPP