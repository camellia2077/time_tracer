// queries/shared/queriers/BaseQuerier.hpp
#ifndef BASE_QUERIER_HPP
#define BASE_QUERIER_HPP

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
            WHERE )" + get_date_condition_sql() + ";";

        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            bind_sql_parameters(stmt);
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

#endif // BASE_QUERIER_HPP