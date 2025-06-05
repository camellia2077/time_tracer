// This file utilizes utility functions defined in common_utils.h, specifically:
// - ProjectNode structure
// - ProjectTree alias
// - time_format_duration(long long, int)
// - split_string(const std::string&, char)
// Do not redefine these structures or functions within this file

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <sqlite3.h>
#include <map>
#include <algorithm>
#include <numeric>
#include <chrono> // For current date
#include <ctime>   // For date manipulation

// --- Utility Structures ---
struct ProjectNode {
    long long duration = 0;
    std::map<std::string, ProjectNode> children;
};
using ProjectTree = std::map<std::string, ProjectNode>;

// --- Utility Functions ---

// 将秒格式化为 "XhYYm" 或 "YYm"
std::string time_format_duration(long long total_seconds, int avg_days = 1) {
    if (total_seconds == 0) {
        if (avg_days > 1) return "0m (average: 0m/day)";
        return "0m";
    }

    long long seconds_per_day = total_seconds;
    if (avg_days > 0 && avg_days != 1) { // Avoid division by zero, and no need to divide if avg_days is 1
         seconds_per_day = total_seconds / avg_days;
    }


    auto format_single_duration = [](long long s) {
        if (s == 0) return std::string("0m");
        long long h = s / 3600;
        long long m = (s % 3600) / 60;
        std::stringstream ss;
        if (h > 0) {
            ss << h << "h";
        }
        if (m > 0 || h == 0) { // Always show minutes if non-zero or if hours are zero
            ss << m << "m";
        }
        return ss.str();
    };

    std::string main_duration_str = format_single_duration(total_seconds);

    if (avg_days > 1) {
        std::string avg_duration_str = format_single_duration(seconds_per_day);
        main_duration_str += " (average: " + avg_duration_str + "/day)";
    }
    return main_duration_str;
}

// 分割字符串
std::vector<std::string> split_string(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// 递归生成项目层级结构的格式化输出行
std::vector<std::string> generate_sorted_output(const ProjectNode& node, int avg_days = 1, int indent = 0) {
    std::vector<std::string> output_lines;
    std::vector<std::pair<std::string, ProjectNode>> sorted_children;

    for (const auto& pair : node.children) {
        sorted_children.push_back(pair);
    }

    std::sort(sorted_children.begin(), sorted_children.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    std::string indent_str(indent * 2, ' '); // 2 spaces per indent level

    for (const auto& pair : sorted_children) {
        const std::string& name = pair.first;
        const ProjectNode& child_node = pair.second;

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

// 获取父子类别映射
std::map<std::string, std::string> get_parent_map(sqlite3* db) {
    std::map<std::string, std::string> parent_map;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT child, parent FROM parent_child;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string child = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string parent = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            parent_map[child] = parent;
        }
    } else {
        std::cerr << "Failed to prepare statement (get_parent_map): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return parent_map;
}

// 构建项目树 (通用逻辑)
void build_project_tree_from_records(
    ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records, // project_path, duration
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
            std::transform(top_level_display_name.begin(), top_level_display_name.end(), top_level_display_name.begin(), ::toupper);
        }

        tree[top_level_display_name].duration += duration;
        ProjectNode* current_node = &tree[top_level_display_name];

        for (size_t i = 1; i < parts.size(); ++i) {
            current_node->children[parts[i]].duration += duration; // Path represents full duration
            current_node = &current_node->children[parts[i]];
        }
    }
}


// --- Database Query Functions ---

// 获取指定年份每日的学习总时长
std::map<std::string, int> get_study_times(sqlite3* db, const std::string& year) {
    std::map<std::string, int> study_times;
    sqlite3_stmt* stmt;
    std::string sql = "SELECT date, SUM(duration) FROM time_records WHERE SUBSTR(date, 1, 4) = ? AND (project_path = 'study' OR project_path LIKE 'study_%') GROUP BY date;";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, year.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int total_duration = sqlite3_column_int(stmt, 1);
            study_times[date] = total_duration;
        }
    } else {
        std::cerr << "Failed to prepare statement (get_study_times): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return study_times;
}

// 查询并显示指定日期的详细统计信息
void query_day(sqlite3* db, const std::string& date_str) {
    std::cout << "\n--- Daily Report for " << date_str << " ---\n";
    sqlite3_stmt* stmt;

    // 1. Get day's metadata
    std::string status = "N/A", remark = "N/A", getup_time = "N/A";
    long long total_day_duration = 0;

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
    } else {
        std::cerr << "Failed to prepare statement (query_day - days): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    // 2. Get total activity duration for the day
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

    // 3. Get project path and duration for the day
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

    // 4. Build project tree
    ProjectTree project_tree;
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    build_project_tree_from_records(project_tree, records, parent_map);


    // 5. Sort and display
    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair : project_tree) {
        sorted_top_level.push_back(pair);
    }
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b){
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair : sorted_top_level) {
        const std::string& category_name = pair.first;
        const ProjectNode& category_node = pair.second;
        double percentage = (total_day_duration > 0) ? (static_cast<double>(category_node.duration) / total_day_duration * 100.0) : 0.0;
        std::cout << "\n## " << category_name << ": "
                  << time_format_duration(category_node.duration)
                  << " (" << std::fixed << std::setprecision(1) << percentage << "%) ##\n";

        std::vector<std::string> output_lines = generate_sorted_output(category_node, 1); // avg_days = 1 for daily
        for (const auto& line : output_lines) {
            std::cout << line << std::endl;
        }
    }
}


// 日期字符串 "YYYYMMDD" 加/减天数
std::string add_days_to_date_str(std::string date_str, int days) {
    int year = std::stoi(date_str.substr(0, 4));
    int month = std::stoi(date_str.substr(4, 2));
    int day = std::stoi(date_str.substr(6, 2));

    std::tm t{};
    t.tm_year = year - 1900; // Years since 1900
    t.tm_mon = month - 1;    // Months since January
    t.tm_mday = day;         // Day of the month

    // Add days
    t.tm_mday += days;
    std::mktime(&t); // Normalize the date

    std::stringstream ss;
    ss << std::put_time(&t, "%Y%m%d");
    return ss.str();
}

// 获取当前日期 "YYYYMMDD"
std::string get_current_date_str() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d");
    return ss.str();
}

// 查询并显示最近 days_to_query 天的汇总统计信息
void query_period(sqlite3* db, int days_to_query) {
    if (days_to_query <= 0) {
        std::cout << "Number of days to query must be positive.\n";
        return;
    }
    std::string end_date_str = get_current_date_str();
    std::string start_date_str = add_days_to_date_str(end_date_str, -(days_to_query -1)); // Inclusive

    std::cout << "\n--- Period Report: Last " << days_to_query << " days ("
              << start_date_str << " to " << end_date_str << ") ---\n";

    sqlite3_stmt* stmt;
    long long total_period_duration = 0;
    int actual_days_with_time_records = 0;

    // 1. Get all records in the period and sum total duration
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

    // 2. Count actual days with time records
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


    // 3. Day status distribution
    std::map<std::string, int> status_counts;
    int total_days_with_status_info = 0;
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
    
    if (total_days_with_status_info > 0){
        std::cout << "Day Status Distribution (based on " << total_days_with_status_info << " days with status entries):\n";
        for(const auto& pair : status_counts){
            double perc = (static_cast<double>(pair.second) / total_days_with_status_info) * 100.0;
             std::cout << "  - Status '" << pair.first << "': " << pair.second << " days ("
                       << std::fixed << std::setprecision(1) << perc << "%)\n";
        }
    } else {
        std::cout << "No day status entries found in this period.\n";
    }
    std::cout << "-------------------------------------\n";


    // 4. Build project tree for the period
    ProjectTree project_tree;
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    build_project_tree_from_records(project_tree, records_in_period, parent_map);


    // 5. Sort and display
    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair : project_tree) {
        sorted_top_level.push_back(pair);
    }
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b){
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair : sorted_top_level) {
        const std::string& category_name = pair.first;
        const ProjectNode& category_node = pair.second;
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


// 以原始文本格式输出指定日期的所有数据
void query_day_raw(sqlite3* db, const std::string& date_str) {
    std::cout << "\n--- Raw Data for " << date_str << " ---\n";
    sqlite3_stmt* stmt;

    // 1. Get day's metadata
    std::string status = "N/A", remark = "N/A", getup_time = "N/A";
    long long total_day_duration = 0;

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
    } else {
        std::cerr << "Failed to prepare statement (query_day_raw - days): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);

    // 2. Get total activity duration for the day
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

    // 3. Get all records for the day, ordered by start time
    std::string sql_records = "SELECT start, end, project_path FROM time_records WHERE date = ? ORDER BY start;";
    if (sqlite3_prepare_v2(db, sql_records.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_str.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string start_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string end_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string project_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::cout << start_time << "~" << end_time << " " << project_path << std::endl;
        }
    } else {
        std::cerr << "Failed to prepare statement (query_day_raw - records): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
}

// 查询并显示指定月份的汇总统计信息
void query_month_summary(sqlite3* db, const std::string& year_month_str) {
    if (year_month_str.length() != 6 || !std::all_of(year_month_str.begin(), year_month_str.end(), ::isdigit)) {
        std::cout << "Invalid year_month format. Expected YYYYMM (e.g., 202301).\n";
        return;
    }
    std::string date_prefix = year_month_str; // YYYYMM

    std::cout << "\n--- Monthly Summary for " << year_month_str.substr(0,4) << "-" << year_month_str.substr(4,2) << " ---\n";

    sqlite3_stmt* stmt;
    long long total_month_duration = 0;
    int actual_days_with_time_records = 0;

    // 1. Get aggregated project data for the month
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


    // 2. Count actual days with time records in the month
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

    // 3. Build project tree using pre-aggregated monthly data
    ProjectTree project_tree;
    std::map<std::string, std::string> parent_map = get_parent_map(db);
    // build_project_tree_from_records can be reused directly as it sums durations for paths.
    build_project_tree_from_records(project_tree, monthly_project_summary, parent_map);


    // 4. Sort and display
    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair : project_tree) {
        sorted_top_level.push_back(pair);
    }
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b){
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair : sorted_top_level) {
        const std::string& category_name = pair.first;
        const ProjectNode& category_node = pair.second;
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


void print_help() {
    std::cout << "Personal Time Logger Analyzer\n";
    std::cout << "Usage: ./time_tracker <command> [arguments]\n";
    std::cout << "Commands:\n";
    std::cout << "  help                         Show this help message.\n";
    std::cout << "  day <YYYYMMDD>               Show detailed statistics for a specific day.\n";
    std::cout << "                               (e.g., ./time_tracker day 20240115)\n";
    std::cout << "  period <N>                   Show summary for the last N days.\n";
    std::cout << "                               (e.g., ./time_tracker period 7)\n";
    std::cout << "  raw <YYYYMMDD>               Show raw data for a specific day.\n";
    std::cout << "                               (e.g., ./time_tracker raw 20240115)\n";
    std::cout << "  month <YYYYMM>               Show summary for a specific month.\n";
    std::cout << "                               (e.g., ./time_tracker month 202401)\n";
    std::cout << "  studytimes <YYYY>            Show daily study times for a year (raw output).\n";
    std::cout << "                               (e.g., ./time_tracker studytimes 2024)\n";
    std::cout << "\nDatabase file 'time_data.db' must be in the same directory.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    sqlite3* db;
    int rc = sqlite3_open("time_data.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db); // sqlite3_close can be called on a null pointer or a failed open if db is not null
        return 1;
    } else {
        // std::cout << "Opened database successfully\n"; // Optional: for debugging
    }

    std::string command = argv[1];

    if (command == "help") {
        print_help();
    } else if (command == "day" && argc == 3) {
        std::string date_str = argv[2];
        if (date_str.length() == 8 && std::all_of(date_str.begin(), date_str.end(), ::isdigit)){
            query_day(db, date_str);
        } else {
            std::cerr << "Invalid date format for 'day'. Expected YYYYMMDD.\n";
            print_help();
        }
    } else if (command == "period" && argc == 3) {
        try {
            int days = std::stoi(argv[2]);
            if (days > 0) {
                query_period(db, days);
            } else {
                std::cerr << "Number of days for 'period' must be positive.\n";
                 print_help();
            }
        } catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid number for 'period'.\n";
             print_help();
        } catch (const std::out_of_range& oor) {
            std::cerr << "Number for 'period' out of range.\n";
             print_help();
        }
    } else if (command == "raw" && argc == 3) {
         std::string date_str = argv[2];
        if (date_str.length() == 8 && std::all_of(date_str.begin(), date_str.end(), ::isdigit)){
            query_day_raw(db, date_str);
        } else {
            std::cerr << "Invalid date format for 'raw'. Expected YYYYMMDD.\n";
            print_help();
        }
    } else if (command == "month" && argc == 3) {
        std::string year_month_str = argv[2];
        query_month_summary(db, year_month_str); // Validation is inside the function
    } else if (command == "studytimes" && argc == 3) {
        std::string year_str = argv[2];
        if (year_str.length() == 4 && std::all_of(year_str.begin(), year_str.end(), ::isdigit)){
            std::map<std::string, int> times = get_study_times(db, year_str);
            std::cout << "\n--- Daily Study Times for " << year_str << " (seconds) ---\n";
            for (const auto& pair : times) {
                std::cout << pair.first << ": " << pair.second << std::endl;
            }
        } else {
             std::cerr << "Invalid year format for 'studytimes'. Expected YYYY.\n";
             print_help();
        }
    }
    else {
        std::cerr << "Unknown command or incorrect arguments.\n";
        print_help();
    }

    sqlite3_close(db);
    return 0;
}