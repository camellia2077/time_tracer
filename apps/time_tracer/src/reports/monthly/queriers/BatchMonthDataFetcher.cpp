// reports/monthly/queriers/BatchMonthDataFetcher.cpp
#include "BatchMonthDataFetcher.hpp"
#include <stdexcept>
#include <iostream>

BatchMonthDataFetcher::BatchMonthDataFetcher(sqlite3* db) : db_(db) {
    if (db_ == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

std::map<std::string, MonthlyReportData> BatchMonthDataFetcher::fetch_all_data() {
    std::map<std::string, MonthlyReportData> all_months_data;

    // 1. 获取项目统计
    fetch_project_stats(all_months_data);

    // 2. 获取实际天数
    fetch_actual_days(all_months_data);

    return all_months_data;
}

void BatchMonthDataFetcher::fetch_project_stats(std::map<std::string, MonthlyReportData>& all_months_data) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = 
        "SELECT strftime('%Y-%m', date) as ym, project_id, SUM(duration) "
        "FROM time_records "
        "GROUP BY ym, project_id "
        "ORDER BY ym;";

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement for monthly stats.");
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* ym_ptr = sqlite3_column_text(stmt, 0);
        if (!ym_ptr) continue;
        
        std::string ym = reinterpret_cast<const char*>(ym_ptr);
        long long project_id = sqlite3_column_int64(stmt, 1);
        long long duration = sqlite3_column_int64(stmt, 2);

        MonthlyReportData& data = all_months_data[ym];
        if (data.year_month.empty()) {
            data.year_month = ym;
        }
        
        data.project_stats.push_back({project_id, duration});
        data.total_duration += duration;
    }
    sqlite3_finalize(stmt);
}

void BatchMonthDataFetcher::fetch_actual_days(std::map<std::string, MonthlyReportData>& all_months_data) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = 
        "SELECT strftime('%Y-%m', date) as ym, COUNT(DISTINCT date) "
        "FROM time_records "
        "GROUP BY ym;";

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement for monthly actual days.");
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* ym_ptr = sqlite3_column_text(stmt, 0);
        if (!ym_ptr) continue;

        std::string ym = reinterpret_cast<const char*>(ym_ptr);
        int days = sqlite3_column_int(stmt, 1);

        if (all_months_data.count(ym)) {
            all_months_data[ym].actual_days = days;
        }
    }
    sqlite3_finalize(stmt);
}