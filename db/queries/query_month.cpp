#include "query_month.h"
#include "query_utils.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>

// --- MonthlyReportGenerator Class Implementation ---

MonthlyReportGenerator::MonthlyReportGenerator(sqlite3* db, const std::string& year_month)
    : m_db(db), m_year_month(year_month) {}

void MonthlyReportGenerator::generate_report() {
    if (!_validate_input()) {
        std::cout << "Invalid year_month format. Expected YYYYMM.\n";
        return;
    }

    _fetch_data();
    _display_summary();

    if (m_actual_days == 0) {
        std::cout << "No time records found for this month.\n";
        return;
    }

    _display_project_breakdown();
}

bool MonthlyReportGenerator::_validate_input() const {
    return m_year_month.length() == 6 && std::all_of(m_year_month.begin(), m_year_month.end(), ::isdigit);
}

void MonthlyReportGenerator::_fetch_data() {
    sqlite3_stmt* stmt;

    // 获取当月所有记录并计算总时长
    std::string sql_records = "SELECT project_path, duration FROM time_records WHERE SUBSTR(date, 1, 6) = ?;";
    if (sqlite3_prepare_v2(m_db, sql_records.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_year_month.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            m_records.push_back({
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                sqlite3_column_int64(stmt, 1)
            });
            m_total_duration += sqlite3_column_int64(stmt, 1);
        }
    }
    sqlite3_finalize(stmt);

    // 获取有记录的实际天数
    std::string sql_actual_days = "SELECT COUNT(DISTINCT date) FROM time_records WHERE SUBSTR(date, 1, 6) = ?;";
    if (sqlite3_prepare_v2(m_db, sql_actual_days.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_year_month.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            m_actual_days = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
}

void MonthlyReportGenerator::_display_summary() {
    std::cout << "\n--- Monthly Summary for " << m_year_month.substr(0, 4) << "-" << m_year_month.substr(4, 2) << " ---\n";
    if (m_actual_days > 0) {
        std::cout << "Actual Days with Records: " << m_actual_days << std::endl;
        std::cout << "Total Time Recorded: " << time_format_duration(m_total_duration, m_actual_days) << std::endl;
    }
    std::cout << "-------------------------------------\n";
}

void MonthlyReportGenerator::_display_project_breakdown() {
    ProjectTree project_tree;
    std::map<std::string, std::string> parent_map = get_parent_map(m_db);
    build_project_tree_from_records(project_tree, m_records, parent_map);

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
                  << " (" << std::fixed << std::setprecision(1) << percentage << "% of total month) ##\n";

        std::vector<std::string> output_lines = generate_sorted_output(category_node, m_actual_days);
        for (const auto& line : output_lines) {
            std::cout << line << std::endl;
        }
    }
}