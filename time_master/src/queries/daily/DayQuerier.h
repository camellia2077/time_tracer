// queries/report_generators/daily/querier/DayQuerier.h
#ifndef DAILY_REPORT_QUERIER_H
#define DAILY_REPORT_QUERIER_H

#include <sqlite3.h>
#include <string>
#include "queries/shared/DailyReportData.h" 

/**
 * @class DayQuerier
 * @brief Fetches daily report data from the SQLite database for a specific date.
 */
class DayQuerier {
public:
    /**
     * @brief Constructs a DayQuerier.
     * @param db A pointer to the SQLite database connection.
     * @param date The date for which to query the report (YYYYMMDD format).
     */
    explicit DayQuerier(sqlite3* db, const std::string& date);

    /**
     * @brief Executes queries to fetch all data for the daily report.
     * @return A DailyReportData struct populated with the fetched data.
     */
    DailyReportData fetch_data();

private:
    void _fetch_metadata(DailyReportData& data);
    void _fetch_total_duration(DailyReportData& data);
    void _fetch_time_records(DailyReportData& data);

    sqlite3* m_db; // 指向 SQLite 数据库连接的指针，用于执行数据库查询
    const std::string m_date; // 表示要查询日报的日期，格式为 YYYYMMDD
};

#endif // DAILY_REPORT_QUERIER_H