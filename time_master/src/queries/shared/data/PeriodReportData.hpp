// query/period/_period_data/ReportData.h
#ifndef PERIOD_REPORT_DATA_HPP
#define PERIOD_REPORT_DATA_HPP

#include <string>
#include <vector>

// 用于周期报告的数据
struct PeriodReportData {
    int days_to_query;
    std::string start_date;
    std::string end_date;
    long long total_duration = 0;
    int actual_days = 0;
    std::vector<std::pair<std::string, long long>> records;
};


#endif // PERIOD_REPORT_DATA_HPP