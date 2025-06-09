#ifndef QUERY_PERIOD_H
#define QUERY_PERIOD_H

#include <sqlite3.h>
#include <string>

/**
 * @brief Queries and displays a summary report for a recent period of days.
 * @param db A pointer to the SQLite database connection.
 * @param days_to_query The number of days to include in the report.
 */
void query_period(sqlite3* db, int days_to_query);

#endif // QUERY_PERIOD_H