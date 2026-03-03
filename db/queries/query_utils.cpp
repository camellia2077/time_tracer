#include "query_utils.h"
#include "common_utils.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <chrono>
#include <ctime>
// This file provides the implementation for the shared utility functions.
// Recursively generates formatted output for the project hierarchy
std::vector<std::string> generate_sorted_output(const ProjectNode& node, int avg_days, int indent) {
    std::vector<std::string> output_lines;
    std::vector<std::pair<std::string, ProjectNode>> sorted_children;

    for (const auto& pair : node.children) {
        sorted_children.push_back(pair);
    }

    std::sort(sorted_children.begin(), sorted_children.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    std::string indent_str(indent * 2, ' ');

    for (const auto& pair : sorted_children) {
        const std::string& name = pair.first;
        const ProjectNode& child_node = pair.second;

        if (child_node.duration > 0 || !child_node.children.empty()) {
            std::stringstream line_ss;
            line_ss << indent_str << "- " << name << ": " << time_format_duration(child_node.duration, avg_days);// Calls time_format_duration from common_utils.h
            output_lines.push_back(line_ss.str());

            if (!child_node.children.empty()) {
                std::vector<std::string> child_lines = generate_sorted_output(child_node, avg_days, indent + 1);
                output_lines.insert(output_lines.end(), child_lines.begin(), child_lines.end());
            }
        }
    }
    return output_lines;
}

// Gets parent-child category mappings
std::map<std::string, std::string> get_parent_map(sqlite3* db) {
    std::map<std::string, std::string> parent_map;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT child, parent FROM parent_child;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            parent_map[reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))] = 
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        }
    } else {
        std::cerr << "Failed to prepare statement (get_parent_map): " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return parent_map;
}

// Builds the project tree from records
void build_project_tree_from_records(
    ProjectTree& tree,
    const std::vector<std::pair<std::string, long long>>& records,
    const std::map<std::string, std::string>& parent_map)
{
    for (const auto& record : records) {
        const std::string& project_path = record.first;
        long long duration = record.second;

        std::vector<std::string> parts = split_string(project_path, '_');// Calls split_string from common_utils.h
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
            current_node->children[parts[i]].duration += duration;
            current_node = &current_node->children[parts[i]];
        }
    }
}

// Adds or subtracts days from a date string
std::string add_days_to_date_str(std::string date_str, int days) {
    int year = std::stoi(date_str.substr(0, 4));
    int month = std::stoi(date_str.substr(4, 2));
    int day = std::stoi(date_str.substr(6, 2));

    std::tm t{};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day + days;
    std::mktime(&t);

    std::stringstream ss;
    ss << std::put_time(&t, "%Y%m%d");
    return ss.str();
}

// Gets the current date string
std::string get_current_date_str() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d");
    return ss.str();
}