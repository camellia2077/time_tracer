// queries/daily/DayQuerier.cpp
#include "DayQuerier.hpp"
#include <stdexcept>

// --- DayQuerier Class Implementation ---

DayQuerier::DayQuerier(sqlite3* db, const std::string& date)
    : m_db(db), m_date(date) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}


DailyReportData DayQuerier::fetch_data() {
    DailyReportData data;
    data.date = m_date;

    _fetch_metadata(data);
    _fetch_total_duration(data);
    _fetch_sleep_time(data); 

    if (data.total_duration > 0) {
        _fetch_time_records(data);
        _fetch_detailed_records(data); 
    }
    return data;
}

// _fetch_metadata 和 _fetch_total_duration 保持不变
void DayQuerier::_fetch_metadata(DailyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT status, sleep, remark, getup_time, exercise FROM days WHERE date = ?;";
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_date.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            data.metadata.status = std::to_string(sqlite3_column_int(stmt, 0));
            data.metadata.sleep = std::to_string(sqlite3_column_int(stmt, 1));
            
            const unsigned char* r = sqlite3_column_text(stmt, 2);
            if (r) data.metadata.remark = reinterpret_cast<const char*>(r);
            const unsigned char* g = sqlite3_column_text(stmt, 3);
            if (g) data.metadata.getup_time = reinterpret_cast<const char*>(g);
            
            data.metadata.exercise = std::to_string(sqlite3_column_int(stmt, 4));
        }
    }
    sqlite3_finalize(stmt);
}

void DayQuerier::_fetch_total_duration(DailyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT SUM(duration) FROM time_records WHERE date = ?;";
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_date.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            data.total_duration = sqlite3_column_int64(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
}


// --- [核心修改] 重写 _fetch_time_records 使用递归CTE ---
void DayQuerier::_fetch_time_records(DailyReportData& data) {
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
        WHERE tr.date = ?;
    )";

    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_date.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            data.records.push_back({
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                sqlite3_column_int64(stmt, 1)
            });
        }
    }
    sqlite3_finalize(stmt);
}

// --- [核心修改] 重写 _fetch_detailed_records 使用递归CTE ---
void DayQuerier::_fetch_detailed_records(DailyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = R"(
        WITH RECURSIVE project_paths(id, path) AS (
            SELECT id, name FROM projects WHERE parent_id IS NULL
            UNION ALL
            SELECT p.id, pp.path || '_' || p.name
            FROM projects p
            JOIN project_paths pp ON p.parent_id = pp.id
        )
        SELECT tr.start, tr.end, pp.path, tr.duration, tr.activity_remark 
        FROM time_records tr
        JOIN project_paths pp ON tr.project_id = pp.id
        WHERE tr.date = ? 
        ORDER BY tr.logical_id ASC;
    )";
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_date.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            TimeRecord record;
            record.start_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            record.end_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            record.project_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            record.duration_seconds = sqlite3_column_int64(stmt, 3);

            const unsigned char* remark_text = sqlite3_column_text(stmt, 4);
            if (remark_text) {
                record.activityRemark = reinterpret_cast<const char*>(remark_text);
            }
            data.detailed_records.push_back(record);
        }
    }
    sqlite3_finalize(stmt);
}

// --- [核心修改] 重写 _fetch_sleep_time 使用 JOIN ---
void DayQuerier::_fetch_sleep_time(DailyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = R"(
        SELECT SUM(tr.duration) 
        FROM time_records tr
        JOIN projects p ON tr.project_id = p.id
        WHERE tr.date = ? AND p.name = 'sleep';
    )";
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_date.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            data.sleep_time = sqlite3_column_int64(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
}