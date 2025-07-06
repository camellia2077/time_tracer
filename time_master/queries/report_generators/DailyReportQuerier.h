#ifndef DAILY_REPORT_QUERIER_H
#define DAILY_REPORT_QUERIER_H

#include <sqlite3.h>
#include <string>
#include <sstream>
#include "query_data_structs.h" // Assumed to contain DailyReportData definition

/**
 * @class DailyReportQuerier
 * @brief Fetches daily report data from the SQLite database for a specific date.
 */
class DailyReportQuerier {
public:
    /**
     * @brief Constructs a DailyReportQuerier.
     * @param db A pointer to the SQLite database connection.
     * @param date The date for which to query the report (YYYY-MM-DD format).
     */
    explicit DailyReportQuerier(sqlite3* db, const std::string& date);

    /**
     * @brief Executes queries to fetch all data for the daily report.
     * @return A DailyReportData struct populated with the fetched data.
     */
    DailyReportData fetch_data();

private:
    void _fetch_metadata(DailyReportData& data);
    void _fetch_total_duration(DailyReportData& data);
    void _fetch_time_records(DailyReportData& data);

    sqlite3* m_db;
    const std::string m_date;
};

/**
 * @class DailyReportFormatter
 * @brief Formats the DailyReportData into a human-readable string.
 */
class DailyReportFormatter {
public:
    /**
     * @brief Default constructor.
     */
    DailyReportFormatter() = default;

    /**
     * @brief Formats the raw report data into a displayable string.
     * @param data The DailyReportData struct to format.
     * @param db A pointer to the SQLite database (needed for project breakdown).
     * @return A formatted string representing the daily report.
     */
    std::string format_report(const DailyReportData& data, sqlite3* db);

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data);
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db);
};

#endif // DAILY_REPORT_QUERIER_H