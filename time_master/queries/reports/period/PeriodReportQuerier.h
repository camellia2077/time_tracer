#ifndef PERIOD_REPORT_QUERIER_H
#define PERIOD_REPORT_QUERIER_H

#include <sqlite3.h>
#include <string>
#include "queries/_shared/PeriodReportData.h"


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

#endif // PERIOD_REPORT_QUERIER_H