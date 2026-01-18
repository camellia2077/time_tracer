// reports/shared/queriers/base_querier.hpp
#ifndef REPORTS_SHARED_QUERIERS_BASE_QUERIER_HPP_
#define REPORTS_SHARED_QUERIERS_BASE_QUERIER_HPP_

#include <sqlite3.h>
#include <string>
#include <vector>
#include <stdexcept>

template<typename ReportDataType, typename QueryParamType>
class BaseQuerier {
public:
    explicit BaseQuerier(sqlite3* db, QueryParamType param)
        : db_(db), param_(param) {
        if (db_ == nullptr) {
            throw std::invalid_argument("Database connection cannot be null.");
        }
    }

    virtual ~BaseQuerier() = default;

    virtual ReportDataType fetch_data() {
        ReportDataType data;
        if (!_validate_input()) {
            _handle_invalid_input(data);
            return data;
        }
        
        _prepare_data(data);
        // [FIX] Moved _fetch_actual_days to subclasses that actually need it.
        _fetch_records_and_duration(data);

        return data;
    }

protected:
    sqlite3* db_;
    QueryParamType param_;

    virtual std::string get_date_condition_sql() const = 0;
    virtual void bind_sql_parameters(sqlite3_stmt* stmt) const = 0;

    virtual bool _validate_input() const {
        return true;
    }

    // [FIX] Silenced unused parameter warning which was treated as an error.
    virtual void _handle_invalid_input(ReportDataType& /*data*/) const {
        // Default implementation does nothing.
    }
    
    // [FIX] Silenced unused parameter warning.
    virtual void _prepare_data(ReportDataType& /*data*/) const {
        // Default implementation does nothing.
    }

    void _fetch_records_and_duration(ReportDataType& data) {
        sqlite3_stmt* stmt;
        
        // [核心优化]：
        // 1. 移除 WITH RECURSIVE (CTE)
        // 2. 使用 GROUP BY project_id 进行数据库级聚合
        std::string sql = "SELECT project_id, SUM(duration) FROM time_records WHERE " 
                          + get_date_condition_sql() 
                          + " GROUP BY project_id;";

        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            bind_sql_parameters(stmt);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                long long project_id = sqlite3_column_int64(stmt, 0);
                long long total_duration = sqlite3_column_int64(stmt, 1);
                
                // 假设你在 ReportDataType (如 DailyReportData) 中加了一个新字段：
                // std::vector<std::pair<long long, long long>> project_stats;
                data.project_stats.push_back({project_id, total_duration});

                data.total_duration += total_duration;
            }
        }
        sqlite3_finalize(stmt);
    }

    // [FIX] This is now a helper for subclasses, not part of the main fetch_data flow.
    void _fetch_actual_days(ReportDataType& data) {
        sqlite3_stmt* stmt;
        std::string sql = "SELECT COUNT(DISTINCT date) FROM time_records WHERE " + get_date_condition_sql() + ";";
        
        if (sqlite3_prepare_v2(this->db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            bind_sql_parameters(stmt);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                data.actual_days = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);
    }
};

#endif // REPORTS_SHARED_QUERIERS_BASE_QUERIER_HPP_