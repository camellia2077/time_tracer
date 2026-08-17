// infra/insights/data/queriers/daily/daily_querier.cpp
#include "infra/insights/data/queriers/daily/daily_querier.hpp"
#include <sqlite3.h>

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>

#include "infra/insights/data/cache/project_name_cache.hpp"
#include "infra/insights/data/utils/daily_status_utils.hpp"
#include "infra/insights/data/utils/activity_record_mapper.hpp"
#include "infra/insights/data/utils/project_tree_builder.hpp"
#include "infra/insights/data/utils/time_derived_stats.hpp"
#include "infra/schema/day_schema.hpp"
#include "infra/schema/sqlite_schema.hpp"
#include "shared/types/insights_errors.hpp"

namespace {
using tracer::core::infrastructure::insights::data::stats::
    DerivedTimeStatsAggregator;

auto BuildDailyStats(
    const std::vector<std::pair<std::int64_t, std::int64_t>>& project_stats,
    const IProjectInfoProvider& provider)
    -> std::map<std::string, std::int64_t> {
  DerivedTimeStatsAggregator aggregator;
  for (const auto& [project_id, duration_seconds] : project_stats) {
    aggregator.AddPathDuration(
        tracer::core::infrastructure::insights::data::record_mapping::
            JoinProjectPath(provider.GetPathParts(project_id)),
        duration_seconds);
  }
  return aggregator.BuildInsightsStatsMap();
}

}  // namespace

DayQuerier::DayQuerier(sqlite3* sqlite_db, std::string_view date,
                       const DailyStatusConfig* status_config)
    : BaseQuerier(sqlite_db, date), status_config_(status_config) {}

auto DayQuerier::FetchData() -> DailyInsightsData {
  if (!HasAnyDayRows()) {
    throw tracer_core::common::InsightsTargetNotFoundError("day", param_);
  }
  DailyInsightsData data =
      BaseQuerier::FetchData();  // BaseQuerier 填充 data.project_stats
  FetchMetadata(data);

  if (data.activity.occurrence_count > 0 || status_config_ != nullptr) {
    ProjectNameCache name_cache;
    name_cache.EnsureLoaded(db_);
    if (data.activity.occurrence_count > 0) {
      FetchDetailedRecords(data, name_cache);
      data.activity.occurrence_count =
          static_cast<int>(data.detailed_records.size());
      data.stats = BuildDailyStats(data.project_stats, name_cache);
      BuildProjectTreeFromIds(data.project_tree, data.project_stats,
                              name_cache);
    } else if (!data.project_stats.empty()) {
      BuildProjectTreeFromIds(data.project_tree, data.project_stats,
                              name_cache);
    }
    if (status_config_ != nullptr) {
      data.metadata.statuses = BuildStatusValues<InsightsStatusValue>(
          data.project_tree, *status_config_);
    }
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
void DayQuerier::PrepareData(DailyInsightsData& data) const {
  data.date = std::string(this->param_);
}

void DayQuerier::FetchMetadata(DailyInsightsData& data) {
  sqlite3_stmt* stmt;
  std::string sql = std::format(
      "SELECT {}, {}, {} FROM {} WHERE {} = ?;", schema::day::db::kRemark,
      schema::day::db::kGetupTime, schema::day::db::kActivityCount,
      schema::day::db::kTable, schema::day::db::kDate);
  if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, param_.data(), static_cast<int>(param_.size()),
                      SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      const unsigned char* remark_ptr = sqlite3_column_text(stmt, 0);
      if (remark_ptr != nullptr) {
        data.metadata.remark = reinterpret_cast<const char*>(remark_ptr);
      }
      const unsigned char* getup_ptr = sqlite3_column_text(stmt, 1);
      if (getup_ptr != nullptr) {
        data.metadata.getup_time = reinterpret_cast<const char*>(getup_ptr);
      }
      data.activity.occurrence_count = sqlite3_column_int(stmt, 2);
    }
  }
  sqlite3_finalize(stmt);
}

void DayQuerier::FetchDetailedRecords(DailyInsightsData& data,
                                      const IProjectInfoProvider& provider) {
  sqlite3_stmt* stmt = nullptr;
  std::string sql = std::format(
      "SELECT {0}, {1}, {2}, {3}, {4}, {7}, {8} "
      "FROM {5} "
      "WHERE {6} = ? "
      "ORDER BY {7} ASC;",
      schema::time_records::db::kStart, schema::time_records::db::kEnd,
      schema::time_records::db::kProjectId, schema::time_records::db::kDuration,
      schema::time_records::db::kActivityRemark,
      schema::time_records::db::kTable, schema::time_records::db::kDate,
      schema::time_records::db::kLogicalId,
      schema::time_records::db::kRecordKind);
  if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, param_.data(), static_cast<int>(param_.size()),
                      SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      TimeRecord record = tracer::core::infrastructure::insights::data::
          record_mapping::ReadTimeRecord(stmt,
                                         {.start_time = 0,
                                          .end_time = 1,
                                          .project_id = 2,
                                          .duration = 3,
                                          .activity_remark = 4,
                                          .logical_id = 5,
                                          .record_kind = 6},
                                         provider);
      data.detailed_records.push_back(record);
    }
  }
  sqlite3_finalize(stmt);
}

BatchDayDataFetcher::BatchDayDataFetcher(sqlite3* sqlite_db,
                                         IProjectInfoProvider& provider,
                                         const DailyStatusConfig* status_config)
    : db_(sqlite_db), provider_(provider), status_config_(status_config) {
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
      schema::day::db::kMonth, schema::day::db::kRemark,
      schema::day::db::kGetupTime, schema::day::db::kActivityCount);

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
    DailyInsightsData& data = result.data_map[date];
    data.date = date;
    constexpr int kColRemark = 3;
    constexpr int kColGetup = 4;
    constexpr int kColActivityCount = 5;

    const unsigned char* remark_ptr = sqlite3_column_text(stmt, kColRemark);
    data.metadata.remark = (remark_ptr != nullptr)
                               ? reinterpret_cast<const char*>(remark_ptr)
                               : "N/A";
    const unsigned char* getup_ptr = sqlite3_column_text(stmt, kColGetup);
    data.metadata.getup_time = (getup_ptr != nullptr)
                                   ? reinterpret_cast<const char*>(getup_ptr)
                                   : "N/A";
    data.activity.occurrence_count =
        sqlite3_column_int(stmt, kColActivityCount);
  }
  sqlite3_finalize(stmt);
}

void BatchDayDataFetcher::FetchTimeRecords(BatchDataResult& result) {
  sqlite3_stmt* stmt;
  const std::string kSql = std::format(
      "SELECT {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8} "
      "FROM {0} ORDER BY {1} ASC, {7} ASC;",
      schema::time_records::db::kTable, schema::time_records::db::kDate,
      schema::time_records::db::kStart, schema::time_records::db::kEnd,
      schema::time_records::db::kProjectId, schema::time_records::db::kDuration,
      schema::time_records::db::kActivityRemark,
      schema::time_records::db::kLogicalId,
      schema::time_records::db::kRecordKind);

  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for time records.");
  }

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

    DailyInsightsData& data = data_it->second;

    constexpr int kColStart = 1;
    constexpr int kColEnd = 2;
    constexpr int kColProjectId = 3;
    constexpr int kColDuration = 4;
    constexpr int kColActivityRemark = 5;
    constexpr int kColLogicalId = 6;
    constexpr int kColRecordKind = 7;

    TimeRecord record = tracer::core::infrastructure::insights::data::
        record_mapping::ReadTimeRecord(stmt,
                                       {.start_time = kColStart,
                                        .end_time = kColEnd,
                                        .project_id = kColProjectId,
                                        .duration = kColDuration,
                                        .activity_remark = kColActivityRemark,
                                        .logical_id = kColLogicalId,
                                        .record_kind = kColRecordKind},
                                       provider_);
    const std::int64_t project_id = sqlite3_column_int64(stmt, kColProjectId);

    data.detailed_records.push_back(record);
    data.activity.Add(record.duration_seconds);
    data.project_stats.emplace_back(project_id, record.duration_seconds);
  }
  sqlite3_finalize(stmt);

  for (auto& [date, data] : result.data_map) {
    data.activity.occurrence_count =
        static_cast<int>(data.detailed_records.size());
  }

  for (auto& [date, data] : result.data_map) {
    if (data.project_stats.empty()) {
      continue;
    }
    data.stats = BuildDailyStats(data.project_stats, provider_);
    BuildProjectTreeFromIds(data.project_tree, data.project_stats, provider_);
  }

  if (status_config_ != nullptr) {
    for (auto& [date, data] : result.data_map) {
      data.metadata.statuses = BuildStatusValues<InsightsStatusValue>(
          data.project_tree, *status_config_);
    }
  }
}
