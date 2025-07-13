#ifndef MONTHLY_REPORT_QUERIER_H
#define MONTHLY_REPORT_QUERIER_H

#include <sqlite3.h>
#include <string>
#include "query_data_structs.h" // 假设此文件在 report_generators 目录下

// 月报查询器类
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

#endif // MONTHLY_REPORT_QUERIER_H