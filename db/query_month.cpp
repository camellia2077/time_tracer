#include "query_month.h"
#include "query_utils.h"
#include "common_utils.h"

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <map>
#include <algorithm>
#include <cctype>

void query_month_summary(sqlite3* db, const std::string& year_month_str) {
    if (year_month_str.length() != 6 || !std::all_of(year_month_str.begin(), year_month_str.end(), ::isdigit)) {
        std::cout << "Invalid year_month format. Expected YYYYMM.\n";
        return;
    }
    std::string date_prefix = year_month_str;

    std::cout << "\n--- Monthly Summary for " << year_month_str.substr(0,4) << "-" << year_month_str.substr(4,2) << " ---\n";

    sqlite3_stmt* stmt;
    long long total_month_duration = 0;
    int actual_days_with_records = 0;

    // Fetch all raw records for the month
    std::vector<std::pair<std::string, long long>> monthly_records;
    std::string sql_records = "SELECT project_path, duration FROM time_records WHERE SUBSTR(date, 1, 6) = ?;";
    if (sqlite3_prepare_v2(db, sql_records.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_prefix.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            monthly_records.push_back({
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                sqlite3_column_int64(stmt, 1)
            });
            total_month_duration += sqlite3_column_int64(stmt, 1);
        }
    }
    sqlite3_finalize(stmt);

    // Count distinct days with records
    std::string sql_actual_days = "SELECT COUNT(DISTINCT date) FROM time_records WHERE SUBSTR(date, 1, 6) = ?;";
    if (sqlite3_prepare_v2(db, sql_actual_days.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_prefix.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            actual_days_with_records = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    if (actual_days_with_records == 0) {
        std::cout << "No time records found for this month.\n";
        return;
    }

    std::cout << "Actual Days with Records: " << actual_days_with_records << std::endl;
    std::cout << "Total Time Recorded: " << time_format_duration(total_month_duration, actual_days_with_records) << std::endl;
    std::cout << "-------------------------------------\n";

    // Build and display the project tree
    ProjectTree project_tree;
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    build_project_tree_from_records(project_tree, monthly_records, parent_map);

    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair : project_tree) sorted_top_level.push_back(pair);
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b){
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair : sorted_top_level) {
        const std::string& category_name = pair.first;
        const ProjectNode& category_node = pair.second;
        double percentage = (total_month_duration > 0) ? (static_cast<double>(category_node.duration) / total_month_duration * 100.0) : 0.0;

        std::cout << "\n## " << category_name << ": "
                  << time_format_duration(category_node.duration, actual_days_with_records)
                  << " (" << std::fixed << std::setprecision(1) << percentage << "% of total month) ##\n";

        std::vector<std::string> output_lines = generate_sorted_output(category_node, actual_days_with_records);
        for (const auto& line : output_lines) std::cout << line << std::endl;
    }
}