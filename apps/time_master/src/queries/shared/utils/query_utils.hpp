// queries/shared/utils/query_utils.hpp
#ifndef QUERY_UTILS_HPP
#define QUERY_UTILS_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "common/common_utils.hpp" // For ProjectNode, ProjectTree
#include "queries/shared/ReportFormat.hpp"


// queries/report_generators/_shared/query_utils.h

// This header declares the shared utility functions.


// --- Standalone Utility Functions ---

// Fetches parent-child category mappings from the database.
std::map<std::string, std::string> get_parent_map(sqlite3* db);

std::string generate_project_breakdown(
    ReportFormat format,
    sqlite3* db,
    const std::vector<std::pair<std::string, long long>>& records,
    long long total_duration,
    int avg_days
);

// Builds a hierarchical project tree from a flat list of time records.
void build_project_tree_from_records(
    ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records,
    const std::map<std::string, std::string>& parent_map);

// Adds or subtracts days from a date string ("YYYYMMDD").
std::string add_days_to_date_str(std::string date_str, int days);

// Gets the current system date as "YYYYMMDD".
std::string get_current_date_str();

#endif // QUERY_UTILS_HPP