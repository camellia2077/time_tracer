#include "query_utils.h"
#include "common/common_utils.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <chrono>
#include <ctime>

// queries/report_generators/_shared/query_utils.cpp

// This file provides the implementation for the shared utility functions.

// --- ProjectBreakdownFormatter Class Implementation ---

/**
 * @brief Constructor for ProjectBreakdownFormatter.
 * Initializes the formatter with the necessary data.
 */
ProjectBreakdownFormatter::ProjectBreakdownFormatter(
    sqlite3* db,
    const std::vector<std::pair<std::string, long long>>& records,
    long long total_duration,
    int avg_days)
    : m_db(db), m_records(records), m_total_duration(total_duration), m_avg_days(avg_days) {}

/**
 * @brief Main method to generate and write the formatted output to a stream.
 * This method orchestrates the fetching of data, building the tree, and formatting the output.
 */
void ProjectBreakdownFormatter::write_to_stream(std::stringstream& ss) {
    // Build the project tree using the provided records.
    std::map<std::string, std::string> parent_map = _get_parent_map();
    _build_project_tree_from_records(parent_map);

    // Sort top-level categories by duration in descending order.
    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair : m_project_tree) {
        sorted_top_level.push_back(pair);
    }
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    // Generate formatted output for each category.
    for (const auto& pair : sorted_top_level) {
        const std::string& category_name = pair.first;
        const ProjectNode& category_node = pair.second;
        double percentage = (m_total_duration > 0) ? (static_cast<double>(category_node.duration) / m_total_duration * 100.0) : 0.0;

        ss << "\n## " << category_name << ": "
           << time_format_duration(category_node.duration, m_avg_days)
           << " (" << std::fixed << std::setprecision(1) << percentage << "%) ##\n";

        std::vector<std::string> output_lines = _generate_sorted_output(category_node);
        for (const auto& line : output_lines) {
            ss << line << "\n";
        }
    }
}

/**
 * @brief Fetches parent-child category mappings from the database.
 * @return A map where the key is the child category and the value is the parent.
 */
std::map<std::string, std::string> ProjectBreakdownFormatter::_get_parent_map() {
    std::map<std::string, std::string> parent_map;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT child, parent FROM parent_child;";
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            parent_map[reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))] =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        }
    } else {
        std::cerr << "Failed to prepare statement (_get_parent_map): " << sqlite3_errmsg(m_db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return parent_map;
}

/**
 * @brief Builds the project tree from the records provided in the constructor.
 * @param parent_map A map to resolve top-level category display names.
 */
void ProjectBreakdownFormatter::_build_project_tree_from_records(const std::map<std::string, std::string>& parent_map) {
    for (const auto& record : m_records) {
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
        }

        m_project_tree[top_level_display_name].duration += duration;
        ProjectNode* current_node = &m_project_tree[top_level_display_name];

        for (size_t i = 1; i < parts.size(); ++i) {
            current_node->children[parts[i]].duration += duration;
            current_node = &current_node->children[parts[i]];
        }
    }
}

/**
 * @brief Recursively generates formatted and sorted output lines for a project node.
 * @param node The current ProjectNode in the hierarchy.
 * @param indent The current indentation level for hierarchical display.
 * @return A vector of formatted strings representing the project breakdown.
 */
std::vector<std::string> ProjectBreakdownFormatter::_generate_sorted_output(const ProjectNode& node, int indent) {
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
            line_ss << indent_str << "- " << name << ": " << time_format_duration(child_node.duration, m_avg_days);
            output_lines.push_back(line_ss.str());

            if (!child_node.children.empty()) {
                std::vector<std::string> child_lines = _generate_sorted_output(child_node, indent + 1);
                output_lines.insert(output_lines.end(), child_lines.begin(), child_lines.end());
            }
        }
    }
    return output_lines;
}


// --- Standalone Utility Functions Implementation ---

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

        std::vector<std::string> parts = split_string(project_path, '_');
        if (parts.empty()) continue;

        std::string top_level_category_key = parts[0];
        std::string top_level_display_name;

        auto it_parent = parent_map.find(top_level_category_key);
        if (it_parent != parent_map.end()) {
            top_level_display_name = it_parent->second;
        } else {
            top_level_display_name = top_level_category_key;
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