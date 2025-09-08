
// queries\daily\DayQuerier.cpp
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

    if (data.total_duration > 0) {
        _fetch_time_records(data);
        _fetch_detailed_records(data); // [新增] 调用新函数
    }
    return data;
}

void DayQuerier::_fetch_metadata(DailyReportData& data) {
    sqlite3_stmt* stmt;
    // [修改] 查询语句中新增 sleep 字段
    std::string sql = "SELECT status, sleep, remark, getup_time FROM days WHERE date = ?;";
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_date.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            data.metadata.status = std::to_string(sqlite3_column_int(stmt, 0));
            // [新增] 读取 sleep 字段并转换为字符串
            data.metadata.sleep = std::to_string(sqlite3_column_int(stmt, 1));
            
            const unsigned char* r = sqlite3_column_text(stmt, 2);
            if (r) data.metadata.remark = reinterpret_cast<const char*>(r);
            const unsigned char* g = sqlite3_column_text(stmt, 3);
            if (g) data.metadata.getup_time = reinterpret_cast<const char*>(g);
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

void DayQuerier::_fetch_time_records(DailyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT project_path, duration FROM time_records WHERE date = ?;";
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

void DayQuerier::_fetch_detailed_records(DailyReportData& data) {
    sqlite3_stmt* stmt;
    // [新增] 查询详细的时间记录
    std::string sql = "SELECT start, end, project_path FROM time_records WHERE date = ? ORDER BY start ASC;";
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_date.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            data.detailed_records.push_back({
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))
            });
        }
    }
    sqlite3_finalize(stmt);
}