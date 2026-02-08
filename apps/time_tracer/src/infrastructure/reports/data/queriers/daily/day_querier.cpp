// infrastructure/reports/data/queriers/daily/day_querier.cpp
#include "infrastructure/reports/data/queriers/daily/day_querier.hpp"

#include <format>
#include <stdexcept>

#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/time_records_schema.hpp"
#include "infrastructure/reports/data/cache/project_name_cache.hpp"
#include "infrastructure/reports/data/utils/project_tree_builder.hpp"

DayQuerier::DayQuerier(sqlite3* sqlite_db, std::string_view date)
    : BaseQuerier(sqlite_db, date) {}

auto DayQuerier::FetchData() -> DailyReportData {
  DailyReportData data =
      BaseQuerier::FetchData();  // BaseQuerier 填充 data.project_stats
  FetchMetadata(data);

  if (data.total_duration > 0) {
    FetchDetailedRecords(data);
    FetchGeneratedStats(data);

    // [新增] 获取并确保缓存加载
    auto& name_cache = ProjectNameCache::Instance();
    name_cache.EnsureLoaded(db_);

    // [核心修改] 传入 name_cache 替代 db_
    BuildProjectTreeFromIds(data.project_tree, data.project_stats, name_cache);
  }
  return data;
}

auto DayQuerier::GetDateConditionSql() const -> std::string {
  return std::format("{} = ?", schema::day::db::kDate);
}
void DayQuerier::BindSqlParameters(sqlite3_stmt* stmt) const {
  sqlite3_bind_text(stmt, 1, param_.data(), static_cast<int>(param_.size()),
                    SQLITE_TRANSIENT);
}
void DayQuerier::PrepareData(DailyReportData& data) const {
  data.date = std::string(this->param_);
}

void DayQuerier::FetchMetadata(DailyReportData& data) {
  sqlite3_stmt* stmt;
  std::string sql = std::format(
      "SELECT {}, {}, {}, {}, {} FROM {} WHERE {} = ?;",
      schema::day::db::kStatus, schema::day::db::kSleep,
      schema::day::db::kRemark, schema::day::db::kGetupTime,
      schema::day::db::kExercise, schema::day::db::kTable,
      schema::day::db::kDate);
  if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, param_.data(), static_cast<int>(param_.size()),
                      SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      data.metadata.status = std::to_string(sqlite3_column_int(stmt, 0));
      data.metadata.sleep = std::to_string(sqlite3_column_int(stmt, 1));
      const unsigned char* remark_ptr = sqlite3_column_text(stmt, 2);
      if (remark_ptr != nullptr) {
        data.metadata.remark = reinterpret_cast<const char*>(remark_ptr);
      }
      const unsigned char* getup_ptr = sqlite3_column_text(stmt, 3);
      if (getup_ptr != nullptr) {
        data.metadata.getup_time = reinterpret_cast<const char*>(getup_ptr);
      }
      data.metadata.exercise = std::to_string(sqlite3_column_int(stmt, 4));
    }
  }
  sqlite3_finalize(stmt);
}

void DayQuerier::FetchDetailedRecords(DailyReportData& data) {
  sqlite3_stmt* stmt;
  std::string sql = std::format(
      R"(
        WITH RECURSIVE {12}({0}, {13}) AS (
            SELECT {0}, {1} FROM {10} p WHERE {2} IS NULL
            UNION ALL
            SELECT p.{0}, pp.{13} || '_' || p.{1}
            FROM {10} p
            JOIN {12} pp ON p.{2} = pp.{0}
        )
        SELECT tr.{3}, tr.{4}, pp.{13}, tr.{5}, tr.{6}
        FROM {11} tr
        JOIN {12} pp ON tr.{7} = pp.{0}
        WHERE tr.{8} = ?
        ORDER BY tr.{9} ASC;
    )",
      schema::projects::db::kId, schema::projects::db::kName,
      schema::projects::db::kParentId, schema::time_records::db::kStart,
      schema::time_records::db::kEnd, schema::time_records::db::kDuration,
      schema::time_records::db::kActivityRemark,
      schema::time_records::db::kProjectId, schema::time_records::db::kDate,
      schema::time_records::db::kLogicalId, schema::projects::db::kTable,
      schema::time_records::db::kTable, schema::projects::cte::kProjectPaths,
      schema::projects::cte::kPath);
  if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, param_.data(), static_cast<int>(param_.size()),
                      SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      TimeRecord record;
      record.start_time =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      record.end_time =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      record.project_path =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
      record.duration_seconds = sqlite3_column_int64(stmt, 3);
      const unsigned char* remark_text = sqlite3_column_text(stmt, 4);
      if (remark_text != nullptr) {
        record.activityRemark = reinterpret_cast<const char*>(remark_text);
      }
      data.detailed_records.push_back(record);
    }
  }
  sqlite3_finalize(stmt);
}

void DayQuerier::FetchGeneratedStats(DailyReportData& data) {
  sqlite3_stmt* stmt;
  std::string sql = std::format(
      "SELECT "
      "{}, "
      "{}, {}, {}, "
      "{}, "
      "{}, "
      "{}, {}, {}, "
      "{} "
      "FROM {} WHERE {} = ?;",
      schema::day::db::kSleepTotalTime, schema::day::db::kTotalExerciseTime,
      schema::day::db::kAnaerobicTime, schema::day::db::kCardioTime,
      schema::day::db::kGroomingTime, schema::day::db::kStudyTime,
      schema::day::db::kRecreationTime, schema::day::db::kRecreationZhihuTime,
      schema::day::db::kRecreationBilibiliTime,
      schema::day::db::kRecreationDouyinTime, schema::day::db::kTable,
      schema::day::db::kDate);

  if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, param_.data(), static_cast<int>(param_.size()),
                      SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      int col_count = sqlite3_column_count(stmt);
      for (int i = 0; i < col_count; ++i) {
        const char* col_name = sqlite3_column_name(stmt, i);
        if (col_name != nullptr) {
          long long val = 0;
          if (sqlite3_column_type(stmt, i) != SQLITE_NULL) {
            val = sqlite3_column_int64(stmt, i);
          }
          data.stats[std::string(col_name)] = val;
        }
      }
    }
  }
  sqlite3_finalize(stmt);
}
