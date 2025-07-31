// queries/report_generators/monthly/querier/MonthlyReportQuerier.cpp
#include "MonthlyReportQuerier.h"
#include "queries/shared/query_utils.h" 
#include <algorithm>
#include <cctype>

MonthlyReportQuerier::MonthlyReportQuerier(sqlite3* db, const std::string& year_month)
    : m_db(db), m_year_month(year_month) {}

MonthlyReportData MonthlyReportQuerier::fetch_data() {
    MonthlyReportData data;
    data.year_month = m_year_month;

    if (!_validate_input()) {
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