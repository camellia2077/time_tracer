#ifndef DAILY_REPORT_GENERATOR_H
#define DAILY_REPORT_GENERATOR_H

#include <sqlite3.h>
#include <string>

/**
 * @class DailyReportGenerator
 * @brief Encapsulates the logic for fetching and formatting a daily report.
 * This class provides a simple interface for generating a complete daily report string.
 */
class DailyReportGenerator {
public:
    /**
     * @brief Constructs a DailyReportGenerator.
     * @param db A pointer to the SQLite database connection.
     */
    explicit DailyReportGenerator(sqlite3* db);

    /**
     * @brief Generates a formatted daily report for a specific date.
     * @param date The date for the report in YYYYMMDD format.
     * @return A string containing the formatted daily report.
     */
    std::string generate_report(const std::string& date);

private:
    sqlite3* m_db;
};

#endif // DAILY_REPORT_GENERATOR_H