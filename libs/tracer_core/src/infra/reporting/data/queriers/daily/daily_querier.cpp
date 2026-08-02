// infra/reporting/data/queriers/daily/daily_querier.cpp
#include "infra/reporting/data/queriers/daily/daily_querier.hpp"
#include <sqlite3.h>

#include <algorithm>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>

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

auto SplitParentPath(std::string_view parent) -> std::vector<std::string> {
  std::vector<std::string> parts;
  size_t start = 0;
  while (start <= parent.size()) {
    const size_t separator = parent.find('/', start);
    const size_t end = separator == std::string_view::npos
                           ? parent.size()
                           : separator;
    if (end == start) {
      return {};
    }
    parts.emplace_back(parent.substr(start, end - start));
    if (separator == std::string_view::npos) {
      break;
    }
    start = separator + 1;
  }
  return parts;
}

auto HasParentActivity(
    const std::vector<std::pair<std::int64_t, std::int64_t>>& project_stats,
    const IProjectInfoProvider& provider, std::string_view parent) -> bool {
  const std::vector<std::string> parent_parts = SplitParentPath(parent);
  if (parent_parts.empty()) {
    return false;
  }

  for (const auto& [project_id, duration_seconds] : project_stats) {
    if (duration_seconds <= 0) {
      continue;
    }
    const std::vector<std::string> path_parts =
        provider.GetPathParts(project_id);
    if (path_parts.size() < parent_parts.size()) {
      continue;
    }
    if (std::equal(parent_parts.begin(), parent_parts.end(),
                   path_parts.begin())) {
      return true;
    }
  }
  return false;
}

auto BuildDailyStatusValues(
    const std::vector<std::pair<std::int64_t, std::int64_t>>& project_stats,
    const IProjectInfoProvider& provider, const DailyStatusConfig& config)
    -> std::vector<DailyStatusValue> {
  std::vector<DailyStatusValue> values;
  values.reserve(config.statuses.size());
  for (const auto& status : config.statuses) {
    values.push_back({.id = status.id,
                      .label = status.label,
                      .value = HasParentActivity(project_stats, provider,
                                                  status.parent)});
  }
  return values;
}

auto ParseRecordKind(const unsigned char* value) -> ActivityRecordKind {
  if (value != nullptr &&
      std::string_view(reinterpret_cast<const char*>(value)) == "end_only") {
    return ActivityRecordKind::kEndOnly;
  }
  return ActivityRecordKind::kInterval;
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

}  // namespace

DayQuerier::DayQuerier(sqlite3* sqlite_db, std::string_view date,
                       const DailyStatusConfig* status_config)
    : BaseQuerier(sqlite_db, date), status_config_(status_config) {}

auto DayQuerier::FetchData() -> DailyReportData {
  if (!HasAnyDayRows()) {
    throw tracer_core::common::ReportTargetNotFoundError("day", param_);
  }
  DailyReportData data =
      BaseQuerier::FetchData();  // BaseQuerier 填充 data.project_stats
  FetchMetadata(data);

  if (data.activity_count > 0 || status_config_ != nullptr) {
    ProjectNameCache name_cache;
    name_cache.EnsureLoaded(db_);
    if (data.activity_count > 0) {
      FetchDetailedRecords(data, name_cache);
      data.activity_count = static_cast<int>(data.detailed_records.size());
      data.stats = BuildDailyStats(data.project_stats, name_cache);
      BuildProjectTreeFromIds(data.project_tree, data.project_stats,
                              name_cache);
    }
    if (status_config_ != nullptr) {
      data.metadata.statuses =
          BuildDailyStatusValues(data.project_stats, name_cache,
                                 *status_config_);
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
void DayQuerier::PrepareData(DailyReportData& data) const {
  data.date = std::string(this->param_);
}

void DayQuerier::FetchMetadata(DailyReportData& data) {
  sqlite3_stmt* stmt;
  std::string sql = std::format(
      "SELECT {}, {}, {} FROM {} WHERE {} = ?;",
      schema::day::db::kRemark, schema::day::db::kGetupTime,
      schema::day::db::kActivityCount,
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
      data.activity_count = sqlite3_column_int(stmt, 2);
    }
  }
  sqlite3_finalize(stmt);
}

void DayQuerier::FetchDetailedRecords(DailyReportData& data,
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
      TimeRecord record;
      record.start_time =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      record.end_time =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      record.logical_id = sqlite3_column_int64(stmt, 5);
      record.kind = ParseRecordKind(sqlite3_column_text(stmt, 6));
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

BatchDayDataFetcher::BatchDayDataFetcher(
    sqlite3* sqlite_db, IProjectInfoProvider& provider,
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
      schema::day::db::kGetupTime,
      schema::day::db::kActivityCount);

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
    data.activity_count = sqlite3_column_int(stmt, kColActivityCount);
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

    DailyReportData& data = data_it->second;

    TimeRecord record;
    constexpr int kColStart = 1;
    constexpr int kColEnd = 2;
    constexpr int kColProjectId = 3;
    constexpr int kColDuration = 4;
    constexpr int kColActivityRemark = 5;
    constexpr int kColLogicalId = 6;
    constexpr int kColRecordKind = 7;

    record.start_time =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColStart));
    record.end_time =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, kColEnd));
    record.kind = ParseRecordKind(sqlite3_column_text(stmt, kColRecordKind));
    record.logical_id = sqlite3_column_int64(stmt, kColLogicalId);
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
    data.project_stats.emplace_back(project_id, record.duration_seconds);
  }
  sqlite3_finalize(stmt);

  for (auto& [date, data] : result.data_map) {
    data.activity_count = static_cast<int>(data.detailed_records.size());
  }

  for (auto& [date, data] : result.data_map) {
    if (data.project_stats.empty()) {
      continue;
    }
    data.stats = BuildDailyStats(data.project_stats, provider_);
  }

  if (status_config_ != nullptr) {
    for (auto& [date, data] : result.data_map) {
      data.metadata.statuses =
          BuildDailyStatusValues(data.project_stats, provider_,
                                 *status_config_);
    }
  }
}
