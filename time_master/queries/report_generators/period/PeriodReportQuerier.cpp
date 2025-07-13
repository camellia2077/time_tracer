#include "PeriodReportQuerier.h"
#include "report_generators/query_utils.h"
#include <iomanip>

// --- PeriodReportQuerier Class Implementation ---

PeriodReportQuerier::PeriodReportQuerier(sqlite3* db, int days_to_query)
    : m_db(db), m_days_to_query(days_to_query) {}

PeriodReportData PeriodReportQuerier::fetch_data() {
    PeriodReportData data;
    data.days_to_query = m_days_to_query;

    if (!_validate_input()) {
        data.days_to_query = -1; // 用负数表示输入无效
        return data;
    }

    // 计算日期范围
    data.end_date = get_current_date_str();
    data.start_date = add_days_to_date_str(data.end_date, -(m_days_to_query - 1));
    
    _fetch_records_and_duration(data);
    _fetch_actual_days(data);
    return data;
}

bool PeriodReportQuerier::_validate_input() const {
    return m_days_to_query > 0;
}

void PeriodReportQuerier::_fetch_records_and_duration(PeriodReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT project_path, duration FROM time_records WHERE date >= ? AND date <= ?;";
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, data.start_date.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, data.end_date.c_str(), -1, SQLITE_STATIC);
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

void PeriodReportQuerier::_fetch_actual_days(PeriodReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT COUNT(DISTINCT date) FROM time_records WHERE date >= ? AND date <= ?;";
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, data.start_date.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, data.end_date.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            data.actual_days = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
}


// --- PeriodReportFormatter Class Implementation ---

std::string PeriodReportFormatter::format_report(const PeriodReportData& data, sqlite3* db) {
    std::stringstream ss;
    if (data.days_to_query <= 0) {
        ss << "Number of days to query must be positive.\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found in this period.\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void PeriodReportFormatter::_display_summary(std::stringstream& ss, const PeriodReportData& data) {
    ss << "\n--- Period Report: Last " << data.days_to_query << " days ("
       << data.start_date << " to " << data.end_date << ") ---\n";
    if (data.actual_days > 0) {
        ss << "Total Time Recorded: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
        ss << "Actual Days with Records: " << data.actual_days << "\n";
    }
    ss << "-------------------------------------\n";
}

void PeriodReportFormatter::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) {
    write_project_breakdown_to_stream(ss, db, data.records, data.total_duration, data.actual_days);
}