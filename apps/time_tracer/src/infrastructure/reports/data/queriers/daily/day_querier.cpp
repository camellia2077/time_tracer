// infrastructure/reports/data/queriers/daily/day_querier.cpp
#include "infrastructure/reports/data/queriers/daily/day_querier.hpp"

#include <stdexcept>

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
  return "date = ?";
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
  std::string sql =
      "SELECT status, sleep, remark, getup_time, exercise FROM days WHERE date "
      "= ?;";
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
  std::string sql = R"(
        WITH RECURSIVE project_paths(id, path) AS (
            SELECT id, name FROM projects WHERE parent_id IS NULL
            UNION ALL
            SELECT p.id, pp.path || '_' || p.name
            FROM projects p
            JOIN project_paths pp ON p.parent_id = pp.id
        )
        SELECT tr.start, tr.end, pp.path, tr.duration, tr.activity_remark
        FROM time_records tr
        JOIN project_paths pp ON tr.project_id = pp.id
        WHERE tr.date = ?
        ORDER BY tr.logical_id ASC;
    )";
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
  std::string sql =
      "SELECT "
      "sleep_total_time, "
      "total_exercise_time, anaerobic_time, cardio_time, "
      "grooming_time, "
      "study_time, "
      "recreation_time, recreation_zhihu_time, recreation_bilibili_time, "
      "recreation_douyin_time "
      "FROM days WHERE date = ?;";

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