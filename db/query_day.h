#ifndef QUERY_DAY_H
#define QUERY_DAY_H

#include <sqlite3.h>
#include <string>

/**
 * @brief Queries and displays a detailed, formatted report for a specific day.
 * @param db A pointer to the SQLite database connection.
 * @param date_str The date to query in "YYYYMMDD" format.
 */
void query_day(sqlite3* db, const std::string& date_str);

#endif // QUERY_DAY_H