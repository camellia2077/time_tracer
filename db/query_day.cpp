#include "query_day.h"
#include "query_utils.h"
#include "common_utils.h"

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <map>
#include <algorithm>

void query_day(sqlite3* db, const std::string& date_str) {
    std::cout << "\n--- Daily Report for " << date_str << " ---\n";
    sqlite3_stmt* stmt;

    std::string status = "N/A", remark = "N/A", getup_time = "N/A";
    long long total_day_duration = 0;

    // Fetch day-specific info
    std::string sql_days = "SELECT status, remark, getup_time FROM days WHERE date = ?;";
    if (sqlite3_prepare_v2(db, sql_days.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_str.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* s = sqlite3_column_text(stmt, 0);
            if (s) status = reinterpret_cast<const char*>(s);
            const unsigned char* r = sqlite3_column_text(stmt, 1);
            if (r) remark = reinterpret_cast<const char*>(r);
            const unsigned char* g = sqlite3_column_text(stmt, 2);
            if (g) getup_time = reinterpret_cast<const char*>(g);
        }
    }
    sqlite3_finalize(stmt);

    // Fetch total duration for the day
    std::string sql_total_duration = "SELECT SUM(duration) FROM time_records WHERE date = ?;";
    if (sqlite3_prepare_v2(db, sql_total_duration.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_str.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            total_day_duration = sqlite3_column_int64(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    std::cout << "Date: " << date_str << std::endl;
    std::cout << "Total Time Recorded: " << time_format_duration(total_day_duration) << std::endl;
    std::cout << "Status: " << status << std::endl;
    std::cout << "Getup Time: " << getup_time << std::endl;
    std::cout << "Remark: " << remark << std::endl;
    std::cout << "-------------------------------------\n";

    if (total_day_duration == 0) {
        std::cout << "No time records for this day.\n";
        return;
    }

    // Fetch all records for the day
    std::vector<std::pair<std::string, long long>> records;
    std::string sql_records = "SELECT project_path, duration FROM time_records WHERE date = ?;";
    if (sqlite3_prepare_v2(db, sql_records.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_str.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            records.push_back({
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                sqlite3_column_int64(stmt, 1)
            });
        }
    }
    sqlite3_finalize(stmt);

    // Build and display the project tree
    ProjectTree project_tree;
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    build_project_tree_from_records(project_tree, records, parent_map);

    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair : project_tree) sorted_top_level.push_back(pair);
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b){
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair : sorted_top_level) {
        const std::string& category_name = pair.first;
        const ProjectNode& category_node = pair.second;
        double percentage = (static_cast<double>(category_node.duration) / total_day_duration * 100.0);
        std::cout << "\n## " << category_name << ": "
                  << time_format_duration(category_node.duration)
                  << " (" << std::fixed << std::setprecision(1) << percentage << "%) ##\n";

        std::vector<std::string> output_lines = generate_sorted_output(category_node, 1);
        for (const auto& line : output_lines) std::cout << line << std::endl;
    }
}