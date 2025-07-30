#ifndef QUERY_UTILS_H
#define QUERY_UTILS_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "common/common_utils.h" // For ProjectNode, ProjectTree


// queries/report_generators/_shared/query_utils.h

// This header declares the shared utility functions.

/**
 * @class ProjectBreakdownFormatter
 * @brief Formats the project breakdown data into a human-readable string stream.
 *
 * This class encapsulates the logic for building a project tree from records,
 * sorting them, and formatting the output into a structured, readable format.
 */
class ProjectBreakdownFormatter {
public:
    /**
     * @brief Constructor.
     * @param db Pointer to the SQLite database, used for fetching category mappings.
     * @param records A vector of pairs, where each pair contains a project path and its duration.
     * @param total_duration The total duration of all records, for percentage calculations.
     * @param avg_days The number of days to average the duration over (default is 1).
     */
    ProjectBreakdownFormatter(
        sqlite3* db,
        const std::vector<std::pair<std::string, long long>>& records,
        long long total_duration,
        int avg_days = 1
    );

    /**
     * @brief Writes the formatted project breakdown to the provided string stream.
     * @param ss The string stream to write the output to.
     */
    void write_to_stream(std::stringstream& ss);

private:
    // Member variables to hold the context for formatting.
    sqlite3* m_db;
    const std::vector<std::pair<std::string, long long>>& m_records;
    long long m_total_duration;
    int m_avg_days;
    ProjectTree m_project_tree; // Internal storage for the constructed project tree.

    // Private helper methods to encapsulate parts of the formatting logic.
    std::map<std::string, std::string> _get_parent_map();
    void _build_project_tree_from_records(const std::map<std::string, std::string>& parent_map);
    std::vector<std::string> _generate_sorted_output(const ProjectNode& node, int indent = 0);
};


// --- Standalone Utility Functions ---

// Fetches parent-child category mappings from the database.
std::map<std::string, std::string> get_parent_map(sqlite3* db);

// Builds a hierarchical project tree from a flat list of time records.
void build_project_tree_from_records(
    ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records,
    const std::map<std::string, std::string>& parent_map);

// Adds or subtracts days from a date string ("YYYYMMDD").
std::string add_days_to_date_str(std::string date_str, int days);

// Gets the current system date as "YYYYMMDD".
std::string get_current_date_str();

#endif // QUERY_UTILS_H