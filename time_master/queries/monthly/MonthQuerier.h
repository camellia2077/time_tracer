// queries/report_generators/monthly/querier/MonthQuerier.h
#ifndef MONTHLY_REPORT_QUERIER_H
#define MONTHLY_REPORT_QUERIER_H

#include <sqlite3.h>
#include <string>
#include "queries/shared/MonthlyReportData.h"


// 月报查询器类
class MonthQuerier {
public:
    explicit MonthQuerier(sqlite3* db, const std::string& year_month);
    MonthlyReportData fetch_data();

private:
    bool _validate_input() const;
    void _fetch_records_and_duration(MonthlyReportData& data);
    void _fetch_actual_days(MonthlyReportData& data);

    sqlite3* m_db;
    const std::string m_year_month;
};

#endif // MONTHLY_REPORT_QUERIER_H