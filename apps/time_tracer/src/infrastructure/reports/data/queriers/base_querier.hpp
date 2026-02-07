// infrastructure/reports/data/queriers/base_querier.hpp
#ifndef REPORTS_DATA_QUERIERS_BASE_QUERIER_H_
#define REPORTS_DATA_QUERIERS_BASE_QUERIER_H_

#include <sqlite3.h>

#include <stdexcept>
#include <string>
#include <vector>

template <typename ReportDataType, typename QueryParamType>
class BaseQuerier {
 public:
  explicit BaseQuerier(sqlite3* sqlite_db, QueryParamType param)
      : db_(sqlite_db), param_(param) {
    if (db_ == nullptr) {
      throw std::invalid_argument("Database connection cannot be null.");
    }
  }

  virtual ~BaseQuerier() = default;

  virtual ReportDataType FetchData() {
    ReportDataType data;
    if (!ValidateInput()) {
      HandleInvalidInput(data);
      return data;
    }

    PrepareData(data);
    // [FIX] Moved FetchActualDays to subclasses that actually need it.
    FetchRecordsAndDuration(data);

    return data;
  }

 protected:
  sqlite3* db_;
  QueryParamType param_;

  [[nodiscard]] virtual std::string GetDateConditionSql() const = 0;
  virtual void BindSqlParameters(sqlite3_stmt* stmt) const = 0;

  [[nodiscard]] virtual bool ValidateInput() const { return true; }

  // [FIX] Silenced unused parameter warning which was treated as an error.
  virtual void HandleInvalidInput(ReportDataType& /*data*/) const {
    // Default implementation does nothing.
  }

  // [FIX] Silenced unused parameter warning.
  virtual void PrepareData(ReportDataType& /*data*/) const {
    // Default implementation does nothing.
  }

  void FetchRecordsAndDuration(ReportDataType& data) {
    sqlite3_stmt* stmt;

    // [核心优化]：
    // 1. 移除 WITH RECURSIVE (CTE)
    // 2. 使用 GROUP BY project_id 进行数据库级聚合
    std::string sql =
        "SELECT project_id, SUM(duration) FROM time_records WHERE " +
        GetDateConditionSql() + " GROUP BY project_id;";

    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
      BindSqlParameters(stmt);
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

  // [FIX] This is now a helper for subclasses, not part of the main FetchData
  // flow.
  void FetchActualDays(ReportDataType& data) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT COUNT(DISTINCT date) FROM time_records WHERE " +
                      GetDateConditionSql() + ";";

    if (sqlite3_prepare_v2(this->db_, sql.c_str(), -1, &stmt, nullptr) ==
        SQLITE_OK) {
      BindSqlParameters(stmt);
      if (sqlite3_step(stmt) == SQLITE_ROW) {
        data.actual_days = sqlite3_column_int(stmt, 0);
      }
    }
    sqlite3_finalize(stmt);
  }
};

#endif  // REPORTS_DATA_QUERIERS_BASE_QUERIER_H_