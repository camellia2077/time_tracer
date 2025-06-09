#include "query_period.h"
#include "query_utils.h"
#include "common_utils.h"

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <map>
#include <algorithm>

void query_period(sqlite3* db, int days_to_query) {
    if (days_to_query <= 0) {
        std::cout << "Number of days to query must be positive.\n";
        return;
    }
    std::string end_date_str = get_current_date_str();
    std::string start_date_str = add_days_to_date_str(end_date_str, -(days_to_query -1));

    std::cout << "\n--- Period Report: Last " << days_to_query << " days ("
              << start_date_str << " to " << end_date_str << ") ---\n";

    sqlite3_stmt* stmt;
    long long total_period_duration = 0;
    int actual_days_with_records = 0;

    // Fetch all records for the period
    std::vector<std::pair<std::string, long long>> records_in_period;
    std::string sql_records = "SELECT project_path, duration FROM time_records WHERE date >= ? AND date <= ?;";
    if (sqlite3_prepare_v2(db, sql_records.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, start_date_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, end_date_str.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            records_in_period.push_back({
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                sqlite3_column_int64(stmt, 1)
            });
            total_period_duration += sqlite3_column_int64(stmt, 1);
        }
    }
    sqlite3_finalize(stmt);

    // Count distinct days with records
    std::string sql_actual_days = "SELECT COUNT(DISTINCT date) FROM time_records WHERE date >= ? AND date <= ?;";
    if (sqlite3_prepare_v2(db, sql_actual_days.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, start_date_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, end_date_str.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            actual_days_with_records = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    if (actual_days_with_records == 0) {
        std::cout << "No time records found in this period.\n";
        return;
    }
    
    std::cout << "Total Time Recorded: " << time_format_duration(total_period_duration, actual_days_with_records) << std::endl;
    std::cout << "Actual Days with Records: " << actual_days_with_records << std::endl;
    std::cout << "-------------------------------------\n";

    // Build and display the project tree
    ProjectTree project_tree;
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    build_project_tree_from_records(project_tree, records_in_period, parent_map);

    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair : project_tree) sorted_top_level.push_back(pair);
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b){
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair : sorted_top_level) {
        const std::string& category_name = pair.first;
        const ProjectNode& category_node = pair.second;
        double percentage = (total_period_duration > 0) ? (static_cast<double>(category_node.duration) / total_period_duration * 100.0) : 0.0;
        
        std::cout << "\n## " << category_name << ": "
                  << time_format_duration(category_node.duration, actual_days_with_records)
                  << " (" << std::fixed << std::setprecision(1) << percentage << "% of total period) ##\n";

        std::vector<std::string> output_lines = generate_sorted_output(category_node, actual_days_with_records);
        for (const auto& line : output_lines) std::cout << line << std::endl;
    }
}