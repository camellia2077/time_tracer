// queries/report_generators/_shared/query_data_structs.h
#ifndef QUERY_DATA_STRUCTS_H
#define QUERY_DATA_STRUCTS_H

#include <string>
#include <vector>
#include <map>

// 用于月报的数据
struct MonthlyReportData {
    std::string year_month;
    long long total_duration = 0;
    int actual_days = 0;
    std::vector<std::pair<std::string, long long>> records;
};

// 用于周期报告的数据
struct PeriodReportData {
    int days_to_query;
    std::string start_date;
    std::string end_date;
    long long total_duration = 0;
    int actual_days = 0;
    std::vector<std::pair<std::string, long long>> records;
};
//  用于导出所有报告的数据结构
using FormattedGroupedReports = std::map<int, std::map<int, std::vector<std::pair<std::string, std::string>>>>;
//  用于导出所有月报的数据结构
using FormattedMonthlyReports = std::map<int, std::map<int, std::string>>;
// 用于导出所有周期报告的数据结构
using FormattedPeriodReports = std::map<int, std::string>;

#endif // QUERY_DATA_STRUCTS_H