#ifndef QUERY_DATA_STRUCTS_H
#define QUERY_DATA_STRUCTS_H

#include <string>
#include <vector>
#include <map>

// This struct is part of the data for a daily report, so it belongs here.
struct DayMetadata {
    std::string status = "N/A";
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
};


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
