#include "monthly_report_querier.h"
#include "query_utils.h"
#include <iomanip>
#include <algorithm>
#include <cctype>

// --- MonthlyReportQuerier Class Implementation ---

MonthlyReportQuerier::MonthlyReportQuerier(sqlite3* db, const std::string& year_month)
    : m_db(db), m_year_month(year_month) {}

MonthlyReportData MonthlyReportQuerier::fetch_data() {
    MonthlyReportData data;
    data.year_month = m_year_month;

    if (!_validate_input()) {
        // 返回一个特殊状态或空数据来表示输入无效
        data.year_month = "INVALID"; 
        return data;
    }

    _fetch_records_and_duration(data);
    _fetch_actual_days(data);
    return data;
}

bool MonthlyReportQuerier::_validate_input() const {
    return m_year_month.length() == 6 && std::all_of(m_year_month.begin(), m_year_month.end(), ::isdigit);
}

void MonthlyReportQuerier::_fetch_records_and_duration(MonthlyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT project_path, duration FROM time_records WHERE SUBSTR(date, 1, 6) = ?;";
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_year_month.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            long long duration = sqlite3_column_int64(stmt, 1);
            data.records.push_back({
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                duration
            });
            data.total_duration += duration;
        }
    }
    sqlite3_finalize(stmt);
}

void MonthlyReportQuerier::_fetch_actual_days(MonthlyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT COUNT(DISTINCT date) FROM time_records WHERE SUBSTR(date, 1, 6) = ?;";
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_year_month.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            data.actual_days = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
}


// --- MonthlyReportFormatter Class Implementation ---

std::string MonthlyReportFormatter::format_report(const MonthlyReportData& data, sqlite3* db) {
    std::stringstream ss;
    if (data.year_month == "INVALID") {
        ss << "Invalid year_month format. Expected YYYYMM.\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found for this month.\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void MonthlyReportFormatter::_display_summary(std::stringstream& ss, const MonthlyReportData& data) {
    ss << "\n--- Monthly Summary for " << data.year_month.substr(0, 4) << "-" << data.year_month.substr(4, 2) << " ---\n";
    if (data.actual_days > 0) {
        ss << "Actual Days with Records: " << data.actual_days << "\n";
        ss << "Total Time Recorded: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
    }
    ss << "-------------------------------------\n";
}

void MonthlyReportFormatter::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) {
    write_project_breakdown_to_stream(ss, db, data.records, data.total_duration, data.actual_days);
}