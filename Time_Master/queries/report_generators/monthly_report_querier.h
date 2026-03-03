#ifndef MONTHLY_REPORT_QUERIER_H
#define MONTHLY_REPORT_QUERIER_H

#include <sqlite3.h>
#include <string>
#include <sstream>
#include "query_data_structs.h"

// 1. 月报查询器类
class MonthlyReportQuerier {
public:
    explicit MonthlyReportQuerier(sqlite3* db, const std::string& year_month);
    MonthlyReportData fetch_data();

private:
    bool _validate_input() const;
    void _fetch_records_and_duration(MonthlyReportData& data);
    void _fetch_actual_days(MonthlyReportData& data);

    sqlite3* m_db;
    const std::string m_year_month;
};

// 2. 月报格式化器类
class MonthlyReportFormatter {
public:
    std::string format_report(const MonthlyReportData& data, sqlite3* db);

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data);
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db);
};

#endif // MONTHLY_REPORT_QUERIER_H