#ifndef DAILY_REPORT_FORMATTER_H
#define DAILY_REPORT_FORMATTER_H

#include <sqlite3.h>
#include <string>
#include <sstream>
#include "report_generators/query_data_structs.h" // Assumed to contain DailyReportData

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

#endif // DAILY_REPORT_FORMATTER_H