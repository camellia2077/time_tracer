// queries/monthly/MonthQuerier.cpp
#include "MonthQuerier.hpp"
#include "queries/shared/utils/query_utils.hpp" 
#include <algorithm>
#include <cctype>

MonthQuerier::MonthQuerier(sqlite3* db, const std::string& year_month)
    : m_db(db), m_year_month(year_month) {}

MonthlyReportData MonthQuerier::fetch_data() {
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

bool MonthQuerier::_validate_input() const {
    return m_year_month.length() == 6 && std::all_of(m_year_month.begin(), m_year_month.end(), ::isdigit);
}

// --- [核心修改] 使用递归CTE重写SQL查询 ---
void MonthQuerier::_fetch_records_and_duration(MonthlyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = R"(
        WITH RECURSIVE project_paths(id, path) AS (
            SELECT id, name FROM projects WHERE parent_id IS NULL
            UNION ALL
            SELECT p.id, pp.path || '_' || p.name
            FROM projects p
            JOIN project_paths pp ON p.parent_id = pp.id
        )
        SELECT pp.path, tr.duration 
        FROM time_records tr
        JOIN project_paths pp ON tr.project_id = pp.id
        WHERE SUBSTR(tr.date, 1, 6) = ?;
    )";

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

void MonthQuerier::_fetch_actual_days(MonthlyReportData& data) {
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