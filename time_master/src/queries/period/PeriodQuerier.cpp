
#include "PeriodQuerier.h"
#include "queries/shared/utils/query_utils.h"
#include <iomanip>

PeriodQuerier::PeriodQuerier(sqlite3* db, int days_to_query)
    : m_db(db), m_days_to_query(days_to_query) {}

PeriodReportData PeriodQuerier::fetch_data() {
    PeriodReportData data;
    data.days_to_query = m_days_to_query;

    if (!_validate_input()) {
        data.days_to_query = -1; // Use a negative number to indicate invalid input
        return data;
    }

    // Calculate date range
    data.end_date = get_current_date_str();
    data.start_date = add_days_to_date_str(data.end_date, -(m_days_to_query - 1));
    
    _fetch_records_and_duration(data);
    _fetch_actual_days(data);
    return data;
}

bool PeriodQuerier::_validate_input() const {
    return m_days_to_query > 0;
}

void PeriodQuerier::_fetch_records_and_duration(PeriodReportData& data) {
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

void PeriodQuerier::_fetch_actual_days(PeriodReportData& data) {
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