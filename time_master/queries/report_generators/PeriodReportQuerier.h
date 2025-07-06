#ifndef PERIOD_REPORT_QUERIER_H
#define PERIOD_REPORT_QUERIER_H

#include <sqlite3.h>
#include <string>
#include <sstream>
#include "query_data_structs.h"

// 1. 周期报告查询器类
class PeriodReportQuerier {
public:
    explicit PeriodReportQuerier(sqlite3* db, int days_to_query);
    PeriodReportData fetch_data();

private:
    bool _validate_input() const;
    void _fetch_records_and_duration(PeriodReportData& data);
    void _fetch_actual_days(PeriodReportData& data);

    sqlite3* m_db;
    const int m_days_to_query;
};

// 2. 周期报告格式化器类
class PeriodReportFormatter {
public:
    std::string format_report(const PeriodReportData& data, sqlite3* db);

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data);
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db);
};

#endif // PERIOD_REPORT_QUERIER_H