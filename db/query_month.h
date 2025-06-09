#ifndef QUERY_MONTH_H
#define QUERY_MONTH_H

#include <sqlite3.h>
#include <string>

/**
 * @brief Queries and displays a summary report for a specific month.
 * @param db A pointer to the SQLite database connection.
 * @param year_month_str The month to query in "YYYYMM" format.
 */
void query_month_summary(sqlite3* db, const std::string& year_month_str);

#endif // QUERY_MONTH_H