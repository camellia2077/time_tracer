#ifndef DAILY_REPORT_QUERIER_H
#define DAILY_REPORT_QUERIER_H

#include <sqlite3.h>
#include <string>
#include <sstream>
#include "query_data_structs.h" // Use the central struct definitions

// 1. 查询器类声明
class DailyReportQuerier {
public:
    explicit DailyReportQuerier(sqlite3* db, const std::string& date);
    DailyReportData fetch_data();

private:
    void _fetch_metadata(DailyReportData& data);
    void _fetch_total_duration(DailyReportData& data);
    void _fetch_time_records(DailyReportData& data);

    sqlite3* m_db;
    const std::string m_date;
};

// 2. 格式化器类声明
class DailyReportFormatter {
public:
    DailyReportFormatter() = default;
    std::string format_report(const DailyReportData& data, sqlite3* db);

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data);
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db);
};

#endif // DAILY_REPORT_QUERIER_H