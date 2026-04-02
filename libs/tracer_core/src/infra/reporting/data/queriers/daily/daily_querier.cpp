// infra/reporting/data/queriers/daily/daily_querier.cpp
#include "infra/reporting/data/queriers/daily/daily_querier.hpp"
#include <sqlite3.h>

#include <cstdint>
#include <format>
#include <stdexcept>

#include "infra/reporting/data/cache/project_name_cache.hpp"
#include "infra/reporting/data/utils/project_tree_builder.hpp"
#include "infra/reporting/data/utils/time_derived_stats.hpp"
#include "infra/schema/day_schema.hpp"
#include "infra/schema/sqlite_schema.hpp"
#include "shared/types/reporting_errors.hpp"

namespace {
using tracer::core::infrastructure::reports::data::stats::
    DerivedTimeStatsAggregator;

auto JoinPathParts(const std::vector<std::string>& parts) -> std::string {
  if (parts.empty()) {
    return "";
  }
  std::string path = parts[0];
  for (size_t i = 1; i < parts.size(); ++i) {
    path += "_" + parts[i];
  }
  return path;
}

auto BuildDailyStats(
    const std::vector<std::pair<std::int64_t, std::int64_t>>& project_stats,
    const IProjectInfoProvider& provider)
    -> std::map<std::string, std::int64_t> {
  DerivedTimeStatsAggregator aggregator;
  for (const auto& [project_id, duration_seconds] : project_stats) {
    aggregator.AddPathDuration(JoinPathParts(provider.GetPathParts(project_id)),
                               duration_seconds);
  }
  return aggregator.BuildReportStatsMap();
}

auto BuildDailyFlags(
    const std::vector<std::pair<std::int64_t, std::int64_t>>& project_stats,
    const IProjectInfoProvider& provider)
    -> std::pair<std::string, std::string> {
  DerivedTimeStatsAggregator aggregator;
  for (const auto& [project_id, duration_seconds] : project_stats) {
    aggregator.AddPathDuration(JoinPathParts(provider.GetPathParts(project_id)),
                               duration_seconds);
  }
  return {
      aggregator.HasStudyActivity() ? "1" : "0",
      aggregator.HasExerciseActivity() ? "1" : "0",
  };
}
}  // namespace

DayQuerier::DayQuerier(sqlite3* sqlite_db, std::string_view date)
    : BaseQuerier(sqlite_db, date) {}

auto DayQuerier::FetchData() -> DailyReportData {
  if (!HasAnyDayRows()) {
    throw tracer_core::common::ReportTargetNotFoundError("day", param_);
  }
  DailyReportData data =
      BaseQuerier::FetchData();  // BaseQuerier 填充 data.project_stats
  FetchMetadata(data);
  data.metadata.status = "0";
  data.metadata.exercise = "0";

  if (data.total_duration > 0) {
    ProjectNameCache name_cache;
    name_cache.EnsureLoaded(db_);
    FetchDetailedRecords(data, name_cache);
    data.stats = BuildDailyStats(data.project_stats, name_cache);
    const auto [status, exercise] =
        BuildDailyFlags(data.project_stats, name_cache);
    data.metadata.status = status;
    data.metadata.exercise = exercise;
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
      "SELECT {}, {}, {} FROM {} WHERE {} = ?;", schema::day::db::kWakeAnchor,
      schema::day::db::kRemark, schema::day::db::kGetupTime,
      schema::day::db::kTable, schema::day::db::kDate);
  if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, param_.data(), static_cast<int>(param_.size()),
                      SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      data.metadata.wake_anchor = std::to_string(sqlite3_column_int(stmt, 0));
      const unsigned char* remark_ptr = sqlite3_column_text(stmt, 1);
      if (remark_ptr != nullptr) {
        data.metadata.remark = reinterpret_cast<const char*>(remark_ptr);
      }
      const unsigned char* getup_ptr = sqlite3_column_text(stmt, 2);
      if (getup_ptr != nullptr) {
        data.metadata.getup_time = reinterpret_cast<const char*>(getup_ptr);
      }
    }
  }
  sqlite3_finalize(stmt);
}

void DayQuerier::FetchDetailedRecords(DailyReportData& data,
                                      const IProjectInfoProvider& provider) {
  sqlite3_stmt* stmt = nullptr;
  std::string sql = std::format(
      "SELECT {0}, {1}, {2}, {3}, {4} "
      "FROM {5} "
      "WHERE {6} = ? "
      "ORDER BY {7} ASC;",
      schema::time_records::db::kStart, schema::time_records::db::kEnd,
      schema::time_records::db::kProjectId, schema::time_records::db::kDuration,
      schema::time_records::db::kActivityRemark,
      schema::time_records::db::kTable, schema::time_records::db::kDate,
      schema::time_records::db::kLogicalId);
  if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, param_.data(), static_cast<int>(param_.size()),
                      SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      TimeRecord record;
      record.start_time =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      record.end_time =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      const std::int64_t kProjectId = sqlite3_column_int64(stmt, 2);
      record.project_path = JoinPathParts(provider.GetPathParts(kProjectId));
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

BatchDayDataFetcher::BatchDayDataFetcher(sqlite3* sqlite_db,
                                         IProjectInfoProvider& provider)
    : db_(sqlite_db), provider_(provider) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto BatchDayDataFetcher::FetchAllData() -> BatchDataResult {
  BatchDataResult result;

  provider_.EnsureLoaded(db_);
  FetchDaysMetadata(result);
  FetchTimeRecords(result);

  return result;
}

void BatchDayDataFetcher::FetchDaysMetadata(BatchDataResult& result) {
  sqlite3_stmt* stmt;
  const std::string kSql = std::format(
      "SELECT {1}, {2}, {3}, "
      "{4}, {5}, {6} "
      "FROM {0} ORDER BY {1} ASC;",
      schema::day::db::kTable, schema::day::db::kDate, schema::day::db::kYear,
      schema::day::db::kMonth, schema::day::db::kWakeAnchor,
      schema::day::db::kRemark, schema::day::db::kGetupTime);

  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for days metadata.");
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const char* date_cstr =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    if (date_cstr == nullptr) {
      continue;
    }
    std::string date(date_cstr);
    int year = sqlite3_column_int(stmt, 1);
    int month = sqlite3_column_int(stmt, 2);
    result.date_order.emplace_back(date, year, month);
    DailyReportData& data = result.data_map[date];
    data.date = date;
    constexpr int kColWakeAnchor = 3;
    constexpr int kColRemark = 4;
    constexpr int kColGetup = 5;

    data.metadata.status = "0";
    data.metadata.exercise = "0";
    data.metadata.wake_anchor =
        std::to_string(sqlite3_column_int(stmt, kColWakeAnchor));
    const unsigned char* remark_ptr = sqlite3_column_text(stmt, kColRemark);
    data.metadata.remark = (remark_ptr != nullptr)
                               ? reinterpret_cast<const char*>(remark_ptr)
                               : "N/A";
    const unsigned char* getup_ptr = sqlite3_column_text(stmt, kColGetup);
    data.metadata.getup_time = (getup_ptr != nullptr)
                                   ? reinterpret_cast<const char*>(getup_ptr)
                                   : "N/A";
  }
  sqlite3_finalize(stmt);
}

void BatchDayDataFetcher::FetchTimeRecords(BatchDataResult& result) {
  sqlite3_stmt* stmt;
  const std::string kSql = std::format(
      "SELECT {1}, {2}, {3}, {4}, {5}, {6} "
      "FROM {0} ORDER BY {1} ASC, {7} ASC;",
      schema::time_records::db::kTable, schema::time_records::db::kDate,
      schema::time_records::db::kStart, schema::time_records::db::kEnd,
      schema::time_records::db::kProjectId, schema::time_records::db::kDuration,
      schema::time_records::db::kActivityRemark,
      schema::time_records::db::kLogicalId);

  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for time records.");
  }

  std::map<std::string, std::map<std::int64_t, std::int64_t>> temp_aggregation;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const char* date_cstr =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    if (date_cstr == nullptr) {
      continue;
    }
    std::string date(date_cstr);

    auto data_it = result.data_map.find(date);
    if (data_it == result.data_map.end()) {
      continue;
    }

    DailyReportData& data = data_it->second;

    TimeRecord record;
    constexpr int kColStart = 1;
    constexpr int kColEnd = 2;
    constexpr int kColProjectId = 3;
    constexpr int kColDuration = 4;
    constexpr int kColActivityRemark = 5;

    record.start_time =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColStart));
    record.end_time =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColEnd));
    std::int64_t project_id = sqlite3_column_int64(stmt, kColProjectId);
    record.duration_seconds = sqlite3_column_int64(stmt, kColDuration);
    const unsigned char* remark_ptr =
        sqlite3_column_text(stmt, kColActivityRemark);
    if (remark_ptr != nullptr) {
      record.activityRemark = reinterpret_cast<const char*>(remark_ptr);
    }

    std::vector<std::string> parts = provider_.GetPathParts(project_id);
    record.project_path = JoinPathParts(parts);

    data.detailed_records.push_back(record);
    data.total_duration += record.duration_seconds;

    temp_aggregation[date][project_id] += record.duration_seconds;
  }
  sqlite3_finalize(stmt);

  for (auto& [date, proj_map] : temp_aggregation) {
    auto& data = result.data_map[date];
    data.project_stats.reserve(proj_map.size());
    for (const auto& project_entry : proj_map) {
      data.project_stats.emplace_back(project_entry);
    }
    data.stats = BuildDailyStats(data.project_stats, provider_);
    const auto [status, exercise] =
        BuildDailyFlags(data.project_stats, provider_);
    data.metadata.status = status;
    data.metadata.exercise = exercise;
  }
}
