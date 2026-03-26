// infra/reporting/data/queriers/base_querier.hpp
#ifndef INFRASTRUCTURE_REPORTS_DATA_QUERIERS_BASE_QUERIER_H_
#define INFRASTRUCTURE_REPORTS_DATA_QUERIERS_BASE_QUERIER_H_

#include <sqlite3.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "infra/schema/day_schema.hpp"
#include "infra/schema/sqlite_schema.hpp"

template <typename ReportDataType, typename QueryParamType>
class BaseQuerier {
 public:
  explicit BaseQuerier(sqlite3* sqlite_db, QueryParamType param)
      : db_(sqlite_db), param_(std::move(param)) {
    if (db_ == nullptr) {
      throw std::invalid_argument("Database connection cannot be null.");
    }
  }

  virtual ~BaseQuerier() = default;

  virtual auto FetchData() -> ReportDataType {
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
    int wake_anchor_true_days = 0;
    int exercise_true_days = 0;
    int cardio_true_days = 0;
    int anaerobic_true_days = 0;
  };

  sqlite3* db_;
  QueryParamType param_;

  [[nodiscard]] virtual auto GetDateConditionSql() const -> std::string = 0;
  virtual void BindSqlParameters(sqlite3_stmt* stmt) const = 0;

  [[nodiscard]] virtual auto ValidateInput() const -> bool { return true; }

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

    // Report total_duration is defined as the sum of persisted activity
    // durations. It is not "first start -> last end", so in overnight-heavy
    // day buckets it may legitimately exceed 24 hours.
    std::string sql = "SELECT ";
    sql += schema::time_records::db::kProjectId;
    sql += ", SUM(";
    sql += schema::time_records::db::kDuration;
    sql += ") FROM ";
    sql += schema::time_records::db::kTable;
    sql += " WHERE ";
    sql += GetDateConditionSql();
    sql += " GROUP BY ";
    sql += schema::time_records::db::kProjectId;
    sql += ";";

    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
      BindSqlParameters(stmt);
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::int64_t project_id = sqlite3_column_int64(stmt, 0);
        std::int64_t total_duration = sqlite3_column_int64(stmt, 1);

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
    std::string sql = "SELECT COUNT(DISTINCT ";
    sql += schema::time_records::db::kDate;
    sql += ") FROM ";
    sql += schema::time_records::db::kTable;
    sql += " WHERE ";
    sql += GetDateConditionSql();
    sql += ";";

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
    // Wake-anchor day counts come from persisted day metadata, not from
    // generated overnight sleep activities or arbitrary sleep_* records.
    std::string sql = "SELECT SUM(CASE WHEN ";
    sql += schema::day::db::kWakeAnchor;
    sql +=
        " != 0 THEN 1 ELSE 0 END) "
        "FROM ";
    sql += schema::day::db::kTable;
    sql += " WHERE ";
    sql += GetDateConditionSql();
    sql += ";";

    if (sqlite3_prepare_v2(this->db_, sql.c_str(), -1, &stmt, nullptr) ==
        SQLITE_OK) {
      BindSqlParameters(stmt);
      if (sqlite3_step(stmt) == SQLITE_ROW) {
        counts.wake_anchor_true_days = sqlite3_column_int(stmt, 0);
      }
    }
    sqlite3_finalize(stmt);

    stmt = nullptr;
    std::string record_sql =
        "SELECT "
        "COUNT(DISTINCT CASE WHEN (";
    record_sql += schema::time_records::db::kProjectPathSnapshot;
    record_sql += " = 'study' OR ";
    record_sql += schema::time_records::db::kProjectPathSnapshot;
    record_sql += " LIKE 'study\\_%' ESCAPE '\\') THEN ";
    record_sql += schema::time_records::db::kDate;
    record_sql +=
        " END), "
        "COUNT(DISTINCT CASE WHEN (";
    record_sql += schema::time_records::db::kProjectPathSnapshot;
    record_sql += " = 'exercise' OR ";
    record_sql += schema::time_records::db::kProjectPathSnapshot;
    record_sql += " LIKE 'exercise\\_%' ESCAPE '\\') THEN ";
    record_sql += schema::time_records::db::kDate;
    record_sql +=
        " END), "
        "COUNT(DISTINCT CASE WHEN (";
    record_sql += schema::time_records::db::kProjectPathSnapshot;
    record_sql += " = 'exercise_cardio' OR ";
    record_sql += schema::time_records::db::kProjectPathSnapshot;
    record_sql += " LIKE 'exercise_cardio\\_%' ESCAPE '\\') THEN ";
    record_sql += schema::time_records::db::kDate;
    record_sql +=
        " END), "
        "COUNT(DISTINCT CASE WHEN (";
    record_sql += schema::time_records::db::kProjectPathSnapshot;
    record_sql += " = 'exercise_anaerobic' OR ";
    record_sql += schema::time_records::db::kProjectPathSnapshot;
    record_sql += " LIKE 'exercise_anaerobic\\_%' ESCAPE '\\') THEN ";
    record_sql += schema::time_records::db::kDate;
    record_sql +=
        " END) "
        "FROM ";
    record_sql += schema::time_records::db::kTable;
    record_sql += " WHERE ";
    record_sql += GetDateConditionSql();
    record_sql += ";";

    if (sqlite3_prepare_v2(this->db_, record_sql.c_str(), -1, &stmt, nullptr) ==
        SQLITE_OK) {
      BindSqlParameters(stmt);
      if (sqlite3_step(stmt) == SQLITE_ROW) {
        counts.status_true_days = sqlite3_column_int(stmt, 0);
        counts.exercise_true_days = sqlite3_column_int(stmt, 1);
        counts.cardio_true_days = sqlite3_column_int(stmt, 2);
        counts.anaerobic_true_days = sqlite3_column_int(stmt, 3);
      }
    }
    sqlite3_finalize(stmt);
    return counts;
  }
};

#endif  // INFRASTRUCTURE_REPORTS_DATA_QUERIERS_BASE_QUERIER_H_
