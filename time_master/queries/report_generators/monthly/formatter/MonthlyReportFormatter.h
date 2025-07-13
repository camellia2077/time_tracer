#ifndef MONTHLY_REPORT_FORMATTER_H
#define MONTHLY_REPORT_FORMATTER_H

#include <sqlite3.h>
#include <string>
#include <sstream>
#include "query_data_structs.h" // 假设此文件在 report_generators 目录下

// 月报格式化器类
class MonthlyReportFormatter {
public:
    std::string format_report(const MonthlyReportData& data, sqlite3* db);

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data);
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db);
};

#endif // MONTHLY_REPORT_FORMATTER_H