// infrastructure/reports/data/queriers/base_querier.hpp
#ifndef REPORTS_DATA_QUERIERS_BASE_QUERIER_H_
#define REPORTS_DATA_QUERIERS_BASE_QUERIER_H_

#include <sqlite3.h>

#include <format>
#include <stdexcept>
#include <string>
#include <vector>

#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/time_records_schema.hpp"

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
  struct DayFlagCounts {
    int status_true_days = 0;
    int sleep_true_days = 0;
    int exercise_true_days = 0;
    int cardio_true_days = 0;
    int anaerobic_true_days = 0;
  };

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
    std::string sql = std::format(
        "SELECT {0}, SUM({1}) FROM {2} WHERE {3} GROUP BY {0};",
        schema::time_records::db::kProjectId,
        schema::time_records::db::kDuration,
        schema::time_records::db::kTable, GetDateConditionSql());

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
    std::string sql = std::format(
        "SELECT COUNT(DISTINCT {0}) FROM {1} WHERE {2};",
        schema::time_records::db::kDate, schema::time_records::db::kTable,
        GetDateConditionSql());

    if (sqlite3_prepare_v2(this->db_, sql.c_str(), -1, &stmt, nullptr) ==
        SQLITE_OK) {
      BindSqlParameters(stmt);
      if (sqlite3_step(stmt) == SQLITE_ROW) {
        data.actual_days = sqlite3_column_int(stmt, 0);
      }
    }
    sqlite3_finalize(stmt);
  }

  [[nodiscard]] auto FetchDayFlagCounts() const -> DayFlagCounts {
    DayFlagCounts counts{};
    sqlite3_stmt* stmt = nullptr;
    std::string sql = std::format(
        "SELECT "
        "SUM(CASE WHEN {0} != 0 THEN 1 ELSE 0 END), "
        "SUM(CASE WHEN {1} != 0 THEN 1 ELSE 0 END), "
        "SUM(CASE WHEN {2} != 0 THEN 1 ELSE 0 END), "
        "SUM(CASE WHEN {3} > 0 THEN 1 ELSE 0 END), "
        "SUM(CASE WHEN {4} > 0 THEN 1 ELSE 0 END) "
        "FROM {5} WHERE {6};",
        schema::day::db::kStatus, schema::day::db::kSleep,
        schema::day::db::kExercise, schema::day::db::kCardioTime,
        schema::day::db::kAnaerobicTime, schema::day::db::kTable,
        GetDateConditionSql());

    if (sqlite3_prepare_v2(this->db_, sql.c_str(), -1, &stmt, nullptr) ==
        SQLITE_OK) {
      BindSqlParameters(stmt);
      if (sqlite3_step(stmt) == SQLITE_ROW) {
        counts.status_true_days = sqlite3_column_int(stmt, 0);
        counts.sleep_true_days = sqlite3_column_int(stmt, 1);
        counts.exercise_true_days = sqlite3_column_int(stmt, 2);
        counts.cardio_true_days = sqlite3_column_int(stmt, 3);
        counts.anaerobic_true_days = sqlite3_column_int(stmt, 4);
      }
    }
    sqlite3_finalize(stmt);
    return counts;
  }
};

#endif  // REPORTS_DATA_QUERIERS_BASE_QUERIER_H_
