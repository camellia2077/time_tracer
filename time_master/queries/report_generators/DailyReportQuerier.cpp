#include "DailyReportQuerier.h"
#include "query_utils.h" // Assumed to contain time_format_duration and write_project_breakdown_to_stream
#include <iomanip>

// --- DailyReportQuerier Class Implementation ---

DailyReportQuerier::DailyReportQuerier(sqlite3* db, const std::string& date) // 构造函数，需要和类使用相同的名称
    // 成员初始化列表
    : m_db(db),  
    m_date(date) {} // {} 表示所有的初始化工作都在成员初始化列表中完成了，不需要在函数体内部执行额外的逻辑
    

DailyReportData DailyReportQuerier::fetch_data() {
    DailyReportData data;
    data.date = m_date; // 向结构体的std::string date;传入m_date

    // Execute all necessary queries
    _fetch_metadata(data);
    _fetch_total_duration(data);

    // Only fetch time records if there's any duration recorded to save resources
    if (data.total_duration > 0) {
        _fetch_time_records(data);
    }
    return data;
}

/**
* @brief 从数据库中获取指定date的元数据（sstatus, remark, getup_time）。
* @param data 对 DailyReportData 结构体的引用，用于填充获取到的元数据。
*/
void DailyReportQuerier::_fetch_metadata(DailyReportData& data) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT status, remark, getup_time FROM days WHERE date = ?;";
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, m_date.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* s = sqlite3_column_text(stmt, 0);
            if (s) data.metadata.status = reinterpret_cast<const char*>(s);
            const unsigned char* r = sqlite3_column_text(stmt, 1);
            if (r) data.metadata.remark = reinterpret_cast<const char*>(r);
            const unsigned char* g = sqlite3_column_text(stmt, 2);
            if (g) data.metadata.getup_time = reinterpret_cast<const char*>(g);
        }
    }
    sqlite3_finalize(stmt);
}

void DailyReportQuerier::_fetch_total_duration(DailyReportData& data) {
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

void DailyReportQuerier::_fetch_time_records(DailyReportData& data) {
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


// --- DailyReportFormatter Class Implementation ---

std::string DailyReportFormatter::format_report(const DailyReportData& data, sqlite3* db) {
    std::stringstream ss;
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << "No time records for this day.\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void DailyReportFormatter::_display_header(std::stringstream& ss, const DailyReportData& data) {
    ss << "\n--- Daily Report for " << data.date << " ---\n";
    ss << "Date: " << data.date << "\n";
    ss << "Total Time Recorded: " << time_format_duration(data.total_duration) << "\n";
    ss << "Status: " << data.metadata.status << "\n";
    ss << "Getup Time: " << data.metadata.getup_time << "\n";
    ss << "Remark: " << data.metadata.remark << "\n";
    ss << "-------------------------------------\n";
}

void DailyReportFormatter::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) {
    // This function remains coupled to query_utils, but the class itself is clean.
    write_project_breakdown_to_stream(ss, db, data.records, data.total_duration, 1);
}