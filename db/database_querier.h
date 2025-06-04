#ifndef DATABASE_QUERIER_H
#define DATABASE_QUERIER_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <sqlite3.h>
#include <map>
#include <algorithm>
#include <numeric>
#include <chrono> 
#include <ctime>   
#include "common_utils.h" // For ProjectNode, time_format_duration, split_string

// --- Querier Specific Utility Functions ---

// Recursively generates formatted output lines for the project hierarchy.
inline std::vector<std::string> generate_sorted_output(const ProjectNode& node, int avg_days = 1, int indent = 0) {
    std::vector<std::string> output_lines;
    std::vector<std::pair<std::string, ProjectNode>> sorted_children;

    for (const auto& pair_node : node.children) { // Use a different name for pair to avoid conflict
        sorted_children.push_back(pair_node);
    }

    std::sort(sorted_children.begin(), sorted_children.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    std::string indent_str(indent * 2, ' '); 

    for (const auto& pair_node : sorted_children) {
        const std::string& name = pair_node.first;
        const ProjectNode& child_node = pair_node.second;

        if (child_node.duration > 0 || !child_node.children.empty()) {
            std::stringstream line_ss;
            line_ss << indent_str << "- " << name << ": " << time_format_duration(child_node.duration, avg_days);
            output_lines.push_back(line_ss.str());

            if (!child_node.children.empty()) {
                std::vector<std::string> child_lines = generate_sorted_output(child_node, avg_days, indent + 1);
                output_lines.insert(output_lines.end(), child_lines.begin(), child_lines.end());
            }
        }
    }
    return output_lines;
}

// Fetches the parent-child category mappings from the database.
inline std::map<std::string, std::string> get_parent_map(sqlite3* db) {
    std::map<std::string, std::string> parent_map;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT child, parent FROM parent_child;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string child_key = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)); // child is a keyword
            std::string parent_val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)); // parent is a keyword
            parent_map[child_key] = parent_val;
        }
    } else {
        std::cerr << "Failed to prepare statement (get_parent_map): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return parent_map;
}

// Builds a hierarchical project tree from a list of records.
inline void build_project_tree_from_records(
    ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records, 
    const std::map<std::string, std::string>& parent_map)
{
    for (const auto& record : records) {
        const std::string& project_path = record.first;
        long long duration = record.second;

        std::vector<std::string> parts = split_string(project_path, '_');
        if (parts.empty()) continue;

        std::string top_level_category_key = parts[0];
        std::string top_level_display_name;

        auto it_parent = parent_map.find(top_level_category_key);
        if (it_parent != parent_map.end()) {
            top_level_display_name = it_parent->second;
        } else {
            top_level_display_name = top_level_category_key;
            std::transform(top_level_display_name.begin(), top_level_display_name.end(), top_level_display_name.begin(), 
                           [](unsigned char c){ return std::toupper(c); }); // Use lambda for toupper
        }

        tree[top_level_display_name].duration += duration;
        ProjectNode* current_node = &tree[top_level_display_name];

        // The logic from the original querier for sub-projects.
        // If "study_program" has duration X, then "study" gets X, and "program" under "study" also gets X.
        // This means the duration is attributed to each part of the path.
        for (size_t i = 1; i < parts.size(); ++i) {
             // The problem description implies that if project_path is "A_B_C" with duration D,
             // then A gets D, A.B gets D, and A.B.C gets D.
             // The provided code `current_node->children[parts[i]].duration += duration;` reflects this.
            current_node->children[parts[i]].duration += duration; 
            current_node = &current_node->children[parts[i]];
        }
    }
}


// --- Database Query Functions ---

inline std::map<std::string, int> get_study_times(sqlite3* db, const std::string& year) {
    std::map<std::string, int> study_times;
    sqlite3_stmt* stmt;
    std::string sql = "SELECT date, SUM(duration) FROM time_records WHERE SUBSTR(date, 1, 4) = ? AND (project_path = 'study' OR project_path LIKE 'study_%') GROUP BY date;";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, year.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string date_val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int total_duration = sqlite3_column_int(stmt, 1);
            study_times[date_val] = total_duration;
        }
    } else {
        std::cerr << "Failed to prepare statement (get_study_times): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return study_times;
}

inline void query_day(sqlite3* db, const std::string& date_str) {
    std::cout << "\n--- Daily Report for " << date_str << " ---\n";
    sqlite3_stmt* stmt;

    std::string status = "N/A", remark = "N/A", getup_time = "N/A";
    long long total_day_duration = 0;

    std::string sql_days = "SELECT status, remark, getup_time FROM days WHERE date = ?;";
    if (sqlite3_prepare_v2(db, sql_days.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_str.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* s_val = sqlite3_column_text(stmt, 0);
            if (s_val) status = reinterpret_cast<const char*>(s_val);
            const unsigned char* r_val = sqlite3_column_text(stmt, 1);
            if (r_val) remark = reinterpret_cast<const char*>(r_val);
            const unsigned char* g_val = sqlite3_column_text(stmt, 2);
            if (g_val) getup_time = reinterpret_cast<const char*>(g_val);
        } else {
             std::cout << "No entry found in 'days' table for date: " << date_str << std::endl;
        }
    } else {
        std::cerr << "Failed to prepare statement (query_day - days): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    std::string sql_total_duration = "SELECT SUM(duration) FROM time_records WHERE date = ?;";
    if (sqlite3_prepare_v2(db, sql_total_duration.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_str.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            total_day_duration = sqlite3_column_int64(stmt, 0);
        }
    } else {
        std::cerr << "Failed to prepare statement (query_day - total_duration): " << sqlite3_errmsg(db) << std::endl;
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
    } else {
        std::cerr << "Failed to prepare statement (query_day - records): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    ProjectTree project_tree;
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    build_project_tree_from_records(project_tree, records, parent_map);

    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair_node : project_tree) {
        sorted_top_level.push_back(pair_node);
    }
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b){
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair_node : sorted_top_level) {
        const std::string& category_name = pair_node.first;
        const ProjectNode& category_node = pair_node.second;
        double percentage = (total_day_duration > 0) ? (static_cast<double>(category_node.duration) / total_day_duration * 100.0) : 0.0;
        std::cout << "\n## " << category_name << ": "
                  << time_format_duration(category_node.duration)
                  << " (" << std::fixed << std::setprecision(1) << percentage << "%) ##\n";

        std::vector<std::string> output_lines = generate_sorted_output(category_node, 1); 
        for (const auto& line : output_lines) {
            std::cout << line << std::endl;
        }
    }
}

inline std::string add_days_to_date_str(std::string date_str, int days_to_add) { // Renamed 'days' to 'days_to_add'
    int year = std::stoi(date_str.substr(0, 4));
    int month = std::stoi(date_str.substr(4, 2));
    int day_val = std::stoi(date_str.substr(6, 2)); // Renamed 'day' to 'day_val'

    std::tm t{};
    t.tm_year = year - 1900; 
    t.tm_mon = month - 1;    
    t.tm_mday = day_val;         

    t.tm_mday += days_to_add;
    std::mktime(&t); 

    std::stringstream ss;
    ss << std::put_time(&t, "%Y%m%d");
    return ss.str();
}

inline std::string get_current_date_str() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d");
    return ss.str();
}

inline void query_period(sqlite3* db, int days_to_query) {
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
    int actual_days_with_time_records = 0;

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
    } else {
        std::cerr << "Failed to prepare statement (query_period - records): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    std::string sql_actual_days = "SELECT COUNT(DISTINCT date) FROM time_records WHERE date >= ? AND date <= ?;";
    if (sqlite3_prepare_v2(db, sql_actual_days.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, start_date_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, end_date_str.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            actual_days_with_time_records = sqlite3_column_int(stmt, 0);
        }
    } else {
        std::cerr << "Failed to prepare statement (query_period - actual_days): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    if (actual_days_with_time_records == 0) {
        std::cout << "No time records found in this period.\n";
        return;
    }
    
    std::cout << "Total Time Recorded: " << time_format_duration(total_period_duration, actual_days_with_time_records) << std::endl;
    std::cout << "Actual Days with Records: " << actual_days_with_time_records << std::endl;

    std::map<std::string, int> status_counts;
    int total_days_with_status_info = 0; // Based on 'days' table entries, not 'time_records'
    std::string sql_status = "SELECT status, COUNT(*) FROM days WHERE date >= ? AND date <= ? GROUP BY status;";
     if (sqlite3_prepare_v2(db, sql_status.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, start_date_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, end_date_str.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* s_text = sqlite3_column_text(stmt, 0);
            std::string status_val = s_text ? reinterpret_cast<const char*>(s_text) : "NULL_STATUS";
            int count = sqlite3_column_int(stmt, 1);
            status_counts[status_val] = count;
            total_days_with_status_info +=count;
        }
    } else {
        std::cerr << "Failed to prepare statement (query_period - status): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    
    if (total_days_with_status_info > 0){ // Changed condition to total_days_with_status_info
        std::cout << "Day Status Distribution (based on " << total_days_with_status_info << " days with status entries):\n";
        for(const auto& pair_node : status_counts){
            double perc = (static_cast<double>(pair_node.second) / total_days_with_status_info) * 100.0;
             std::cout << "  - Status '" << pair_node.first << "': " << pair_node.second << " days ("
                       << std::fixed << std::setprecision(1) << perc << "%)\n";
        }
    } else {
        std::cout << "No day status entries found in this period.\n";
    }
    std::cout << "-------------------------------------\n";

    ProjectTree project_tree;
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    build_project_tree_from_records(project_tree, records_in_period, parent_map);

    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair_node : project_tree) {
        sorted_top_level.push_back(pair_node);
    }
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b){
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair_node : sorted_top_level) {
        const std::string& category_name = pair_node.first;
        const ProjectNode& category_node = pair_node.second;
        double percentage = (total_period_duration > 0) ? (static_cast<double>(category_node.duration) / total_period_duration * 100.0) : 0.0;
        
        std::cout << "\n## " << category_name << ": "
                  << time_format_duration(category_node.duration, actual_days_with_time_records)
                  << " (" << std::fixed << std::setprecision(1) << percentage << "% of total period) ##\n";

        std::vector<std::string> output_lines = generate_sorted_output(category_node, actual_days_with_time_records);
        for (const auto& line : output_lines) {
            std::cout << line << std::endl;
        }
    }
}

inline void query_day_raw(sqlite3* db, const std::string& date_str) {
    std::cout << "\n--- Raw Data for " << date_str << " ---\n";
    sqlite3_stmt* stmt;

    std::string status = "N/A", remark = "N/A", getup_time = "N/A";
    long long total_day_duration = 0;

    std::string sql_days = "SELECT status, remark, getup_time FROM days WHERE date = ?;";
    if (sqlite3_prepare_v2(db, sql_days.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_str.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* s_val = sqlite3_column_text(stmt, 0); // Renamed to avoid conflict
            if (s_val) status = reinterpret_cast<const char*>(s_val);
            const unsigned char* r_val = sqlite3_column_text(stmt, 1); // Renamed
            if (r_val) remark = reinterpret_cast<const char*>(r_val);
            const unsigned char* g_val = sqlite3_column_text(stmt, 2); // Renamed
            if (g_val) getup_time = reinterpret_cast<const char*>(g_val);
        } else {
            std::cout << "No entry found in 'days' table for date: " << date_str << " (for raw output)" << std::endl;
        }
    } else {
        std::cerr << "Failed to prepare statement (query_day_raw - days): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    std::string sql_total_duration = "SELECT SUM(duration) FROM time_records WHERE date = ?;";
    if (sqlite3_prepare_v2(db, sql_total_duration.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_str.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            total_day_duration = sqlite3_column_int64(stmt, 0);
        }
    } else {
        std::cerr << "Failed to prepare statement (query_day_raw - total_duration): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    std::cout << "Date:" << date_str
              << ", Total Time:" << time_format_duration(total_day_duration)
              << ", Status:" << status
              << ", Getup:" << getup_time
              << ", Remark:" << remark << std::endl;

    std::string sql_records = "SELECT start, end, project_path FROM time_records WHERE date = ? ORDER BY start;";
    if (sqlite3_prepare_v2(db, sql_records.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_str.c_str(), -1, SQLITE_STATIC);
        bool has_records = false;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            has_records = true;
            std::string start_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string end_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string project_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::cout << start_time << "~" << end_time << " " << project_path << std::endl;
        }
        if (!has_records) {
            std::cout << "No time_records entries for this day." << std::endl;
        }
    } else {
        std::cerr << "Failed to prepare statement (query_day_raw - records): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
}

inline void query_month_summary(sqlite3* db, const std::string& year_month_str) {
    if (year_month_str.length() != 6 || !std::all_of(year_month_str.begin(), year_month_str.end(), ::isdigit)) {
        std::cout << "Invalid year_month format. Expected YYYYMM (e.g., 202301).\n";
        return;
    }
    std::string date_prefix = year_month_str; 

    std::cout << "\n--- Monthly Summary for " << year_month_str.substr(0,4) << "-" << year_month_str.substr(4,2) << " ---\n";

    sqlite3_stmt* stmt;
    long long total_month_duration = 0;
    int actual_days_with_time_records = 0;

    std::vector<std::pair<std::string, long long>> monthly_project_summary;
    std::string sql_agg_records = "SELECT project_path, SUM(duration) FROM time_records WHERE SUBSTR(date, 1, 6) = ? GROUP BY project_path;";
    if (sqlite3_prepare_v2(db, sql_agg_records.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_prefix.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string project_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            long long duration_sum = sqlite3_column_int64(stmt, 1);
            monthly_project_summary.push_back({project_path, duration_sum});
            total_month_duration += duration_sum;
        }
    } else {
        std::cerr << "Failed to prepare statement (query_month_summary - agg_records): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    std::string sql_actual_days = "SELECT COUNT(DISTINCT date) FROM time_records WHERE SUBSTR(date, 1, 6) = ?;";
    if (sqlite3_prepare_v2(db, sql_actual_days.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_prefix.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            actual_days_with_time_records = sqlite3_column_int(stmt, 0);
        }
    } else {
        std::cerr << "Failed to prepare statement (query_month_summary - actual_days): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    if (actual_days_with_time_records == 0) {
        std::cout << "No time records found for this month.\n";
        return;
    }

    std::cout << "Month: " << year_month_str.substr(0,4) << "-" << year_month_str.substr(4,2) << std::endl;
    std::cout << "Actual Days with Records: " << actual_days_with_time_records << std::endl;
    std::cout << "Total Time Recorded: " << time_format_duration(total_month_duration, actual_days_with_time_records) << std::endl;
    std::cout << "-------------------------------------\n";

    ProjectTree project_tree;
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    build_project_tree_from_records(project_tree, monthly_project_summary, parent_map);

    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair_node : project_tree) {
        sorted_top_level.push_back(pair_node);
    }
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b){
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair_node : sorted_top_level) {
        const std::string& category_name = pair_node.first;
        const ProjectNode& category_node = pair_node.second;
        double percentage = (total_month_duration > 0) ? (static_cast<double>(category_node.duration) / total_month_duration * 100.0) : 0.0;

        std::cout << "\n## " << category_name << ": "
                  << time_format_duration(category_node.duration, actual_days_with_time_records)
                  << " (" << std::fixed << std::setprecision(1) << percentage << "% of total month) ##\n";

        std::vector<std::string> output_lines = generate_sorted_output(category_node, actual_days_with_time_records);
        for (const auto& line : output_lines) {
            std::cout << line << std::endl;
        }
    }
}

#endif // DATABASE_QUERIER_H