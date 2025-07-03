#include "query_period.h"
#include "query_utils.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

// --- PeriodReportGenerator Class Implementation ---

PeriodReportGenerator::PeriodReportGenerator(sqlite3* db, int days_to_query)
    : m_db(db), m_days_to_query(days_to_query) {}

void PeriodReportGenerator::generate_report() {
    if (!_validate_input()) {
        std::cout << "Number of days to query must be positive.\n";
        return;
    }

    // 计算日期范围
    m_end_date = get_current_date_str();
    m_start_date = add_days_to_date_str(m_end_date, -(m_days_to_query - 1));

    _fetch_data();
    _display_summary();

    if (m_actual_days == 0) {
        std::cout << "No time records found in this period.\n";
        return;
    }

    _display_project_breakdown();
}

bool PeriodReportGenerator::_validate_input() const {
    return m_days_to_query > 0;
}

void PeriodReportGenerator::_fetch_data() {
    sqlite3_stmt* stmt;

    // 获取周期内的所有记录并计算总时长
    std::string sql_records = "SELECT project_path, duration FROM time_records WHERE date >= ? AND date <= ?;";
    if (sqlite3_prepare_v2(m_db, sql_records.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_start_date.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, m_end_date.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            m_records.push_back({
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                sqlite3_column_int64(stmt, 1)
            });
            m_total_duration += sqlite3_column_int64(stmt, 1);
        }
    }
    sqlite3_finalize(stmt);

    // 获取周期内有记录的实际天数
    std::string sql_actual_days = "SELECT COUNT(DISTINCT date) FROM time_records WHERE date >= ? AND date <= ?;";
    if (sqlite3_prepare_v2(m_db, sql_actual_days.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_start_date.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, m_end_date.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            m_actual_days = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
}

void PeriodReportGenerator::_display_summary() {
    std::cout << "\n--- Period Report: Last " << m_days_to_query << " days ("
              << m_start_date << " to " << m_end_date << ") ---\n";
    if (m_actual_days > 0) {
        std::cout << "Total Time Recorded: " << time_format_duration(m_total_duration, m_actual_days) << std::endl;
        std::cout << "Actual Days with Records: " << m_actual_days << std::endl;
    }
    std::cout << "-------------------------------------\n";
}

void PeriodReportGenerator::_display_project_breakdown() {
    ProjectTree project_tree;
    std::map<std::string, std::string> parent_map = get_parent_map(m_db);
    build_project_tree_from_records(project_tree, m_records, parent_map);// Calls build_project_tree_from_records from common_utils.h

    std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
    for (const auto& pair : project_tree) {
        sorted_top_level.push_back(pair);
    }
    std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b) {
        return a.second.duration > b.second.duration;
    });

    for (const auto& pair : sorted_top_level) {
        const std::string& category_name = pair.first;
        const ProjectNode& category_node = pair.second;
        double percentage = (m_total_duration > 0) ? (static_cast<double>(category_node.duration) / m_total_duration * 100.0) : 0.0;

        std::cout << "\n## " << category_name << ": "
                  << time_format_duration(category_node.duration, m_actual_days)
                  << " (" << std::fixed << std::setprecision(1) << percentage << "% of total period) ##\n";

        std::vector<std::string> output_lines = generate_sorted_output(category_node, m_actual_days);// Calls generate_sorted_output from common_utils.h
        for (const auto& line : output_lines) {
            std::cout << line << std::endl;
        }
    }
}