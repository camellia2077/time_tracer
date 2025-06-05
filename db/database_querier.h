#ifndef DATABASE_QUERIER_H
#define DATABASE_QUERIER_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include "common_utils.h" // Provides ProjectNode, ProjectTree, and utility functions

// --- Function Declarations for the Database Querier Interface ---

/**
 * @brief Recursively generates formatted and sorted output lines for the project hierarchy.
 * @param node The current project node to process.
 * @param avg_days The number of days to average the duration over. Defaults to 1.
 * @param indent The current indentation level for hierarchical display. Defaults to 0.
 * @return A vector of strings, each representing a formatted line of output.
 */
std::vector<std::string> generate_sorted_output(const ProjectNode& node, int avg_days = 1, int indent = 0);

/**
 * @brief Fetches the parent-child category mappings from the database.
 * @param db A pointer to the SQLite database connection.
 * @return A map where the key is the child category and the value is the parent category.
 */
std::map<std::string, std::string> get_parent_map(sqlite3* db);

/**
 * @brief Builds a hierarchical project tree from a flat list of time records.
 * @param tree A reference to the ProjectTree to be populated.
 * @param records A vector of pairs, each containing a project_path and its duration.
 * @param parent_map A map of child-to-parent category mappings.
 */
void build_project_tree_from_records(
    ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records,
    const std::map<std::string, std::string>& parent_map);

/**
 * @brief Gets the total study duration for each day of a specified year.
 * @param db A pointer to the SQLite database connection.
 * @param year The year to query (e.g., "2024").
 * @return A map where the key is the date string ("YYYYMMDD") and the value is the total study seconds.
 */
std::map<std::string, int> get_study_times(sqlite3* db, const std::string& year);

/**
 * @brief Queries and displays a detailed, formatted report for a specific day.
 * @param db A pointer to the SQLite database connection.
 * @param date_str The date to query in "YYYYMMDD" format.
 */
void query_day(sqlite3* db, const std::string& date_str);

/**
 * @brief Adds or subtracts a number of days from a date string.
 * @param date_str The starting date in "YYYYMMDD" format.
 * @param days The number of days to add (can be negative).
 * @return The resulting date string in "YYYYMMDD" format.
 */
std::string add_days_to_date_str(std::string date_str, int days);

/**
 * @brief Gets the current system date as a string.
 * @return The current date in "YYYYMMDD" format.
 */
std::string get_current_date_str();

/**
 * @brief Queries and displays a summary report for a recent period of days.
 * @param db A pointer to the SQLite database connection.
 * @param days_to_query The number of days to include in the report (e.g., 7, 30).
 */
void query_period(sqlite3* db, int days_to_query);

/**
 * @brief Queries and displays the raw, unprocessed time log data for a specific day.
 * @param db A pointer to the SQLite database connection.
 * @param date_str The date to query in "YYYYMMDD" format.
 */
void query_day_raw(sqlite3* db, const std::string& date_str);

/**
 * @brief Queries and displays a summary report for a specific month.
 * @param db A pointer to the SQLite database connection.
 * @param year_month_str The month to query in "YYYYMM" format.
 */
void query_month_summary(sqlite3* db, const std::string& year_month_str);

#endif // DATABASE_QUERIER_H