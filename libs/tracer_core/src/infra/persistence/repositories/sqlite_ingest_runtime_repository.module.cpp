#include <sqlite3.h>

#include <format>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "infra/persistence/repositories/sqlite_ingest_runtime_repository.hpp"
#include "infra/persistence/sqlite/db_manager.hpp"
#include "infra/schema/sqlite_schema.hpp"

namespace tracer::core::infrastructure::persistence {
namespace {

using tracer_core::core::dto::IngestSyncStatusEntry;
using tracer_core::core::dto::IngestSyncStatusOutput;
using tracer_core::core::dto::IngestSyncStatusRequest;

auto OpenReadDatabaseOrReturnNull(DBManager& manager) -> sqlite3* {
  if (!manager.OpenDatabaseIfNeeded()) {
    return nullptr;
  }
  return manager.GetDbConnection();
}

auto ReadText(sqlite3_stmt* statement, int column) -> std::string {
  const auto* value = sqlite3_column_text(statement, column);
  return value == nullptr ? std::string{}
                          : reinterpret_cast<const char*>(value);
}

auto QueryLatestActivityTail(
    sqlite3* db_connection, std::string_view date,
    const bool include_requested_date)
    -> std::optional<
        tracer_core::application::ports::ActivityTailQueryResult> {
  const char* const kDateComparator = include_requested_date ? "<=" : "<";
  const std::string sql = std::format(
      "SELECT {0}, \"{1}\" FROM {2} WHERE {0} {3} ?1 "
      "ORDER BY {0} DESC, {4} DESC LIMIT 1;",
      schema::time_records::db::kDate, schema::time_records::db::kEnd,
      schema::time_records::db::kTable, kDateComparator,
      schema::time_records::db::kEndTimestamp);
  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(db_connection, sql.c_str(), -1, &statement, nullptr) !=
      SQLITE_OK) {
    return std::nullopt;
  }
  if (sqlite3_bind_text(statement, 1, date.data(),
                        static_cast<int>(date.size()),
                        SQLITE_TRANSIENT) != SQLITE_OK) {
    sqlite3_finalize(statement);
    return std::nullopt;
  }

  std::optional<
      tracer_core::application::ports::ActivityTailQueryResult> result;
  if (sqlite3_step(statement) == SQLITE_ROW) {
    const std::string result_date = ReadText(statement, 0);
    const std::string end_time = ReadText(statement, 1);
    if (!result_date.empty() && !end_time.empty()) {
      result = tracer_core::application::ports::ActivityTailQueryResult{
          .date = result_date, .end_time = end_time};
    }
  }
  sqlite3_finalize(statement);
  return result;
}

}  // namespace

SqliteIngestRuntimeRepository::SqliteIngestRuntimeRepository(
    std::string db_path)
    : db_path_(std::move(db_path)) {}

auto SqliteIngestRuntimeRepository::ListIngestSyncStatuses(
    const IngestSyncStatusRequest& request) const -> IngestSyncStatusOutput {
  DBManager db_manager(db_path_);
  sqlite3* db_connection = OpenReadDatabaseOrReturnNull(db_manager);
  if (db_connection == nullptr) {
    return {.ok = true, .items = {}, .error_message = ""};
  }

  std::string sql =
      std::format("SELECT {1}, {2}, {3}, {4} FROM {0}",
                  schema::ingest_month_sync::db::kTable,
                  schema::ingest_month_sync::db::kMonthKey,
                  schema::ingest_month_sync::db::kTxtRelativePath,
                  schema::ingest_month_sync::db::kTxtContentHashSha256,
                  schema::ingest_month_sync::db::kIngestedAtUnixMs);
  if (!request.months.empty()) {
    sql += " WHERE ";
    for (size_t index = 0; index < request.months.size(); ++index) {
      if (index > 0U) {
        sql += " OR ";
      }
      sql += std::format("{0} = ?{1}", schema::ingest_month_sync::db::kMonthKey,
                         index + 1U);
    }
  }
  sql += std::format(" ORDER BY {0} ASC;",
                     schema::ingest_month_sync::db::kMonthKey);

  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(db_connection, sql.c_str(), -1, &statement, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error("Prepare ingest sync query failed: " +
                             std::string(sqlite3_errmsg(db_connection)));
  }

  try {
    for (size_t index = 0; index < request.months.size(); ++index) {
      if (sqlite3_bind_text(statement, static_cast<int>(index + 1U),
                            request.months[index].c_str(), -1,
                            SQLITE_TRANSIENT) != SQLITE_OK) {
        throw std::runtime_error("Bind ingest sync query month failed.");
      }
    }

    IngestSyncStatusOutput output{.ok = true, .items = {}, .error_message = ""};
    for (;;) {
      const int step = sqlite3_step(statement);
      if (step == SQLITE_DONE) {
        break;
      }
      if (step != SQLITE_ROW) {
        throw std::runtime_error("Execute ingest sync query failed: " +
                                 std::string(sqlite3_errmsg(db_connection)));
      }
      output.items.push_back(IngestSyncStatusEntry{
          .month_key = ReadText(statement, 0),
          .txt_relative_path = ReadText(statement, 1),
          .txt_content_hash_sha256 = ReadText(statement, 2),
          .ingested_at_unix_ms = sqlite3_column_int64(statement, 3),
      });
    }
    sqlite3_finalize(statement);
    return output;
  } catch (...) {
    sqlite3_finalize(statement);
    throw;
  }
}

auto SqliteIngestRuntimeRepository::TryGetLatestActivityTailBeforeDate(
    std::string_view date) const
    -> std::optional<tracer_core::application::ports::PreviousActivityTail> {
  DBManager db_manager(db_path_);
  sqlite3* db_connection = OpenReadDatabaseOrReturnNull(db_manager);
  if (db_connection == nullptr) {
    return std::nullopt;
  }

  const auto kResult = QueryLatestActivityTail(db_connection, date, false);
  if (!kResult.has_value()) {
    return std::nullopt;
  }
  return tracer_core::application::ports::PreviousActivityTail{
      .date = kResult->date, .end_time = kResult->end_time};
}

auto SqliteIngestRuntimeRepository::TryGetLatestActivityTailAtOrBeforeDate(
    std::string_view date) const
    -> std::optional<
        tracer_core::application::ports::ActivityTailQueryResult> {
  DBManager db_manager(db_path_);
  sqlite3* db_connection = OpenReadDatabaseOrReturnNull(db_manager);
  if (db_connection == nullptr) {
    return std::nullopt;
  }

  return QueryLatestActivityTail(db_connection, date, true);
}

auto SqliteIngestRuntimeRepository::TryGetLatestActivityRecordOnDate(
    std::string_view date) const
    -> std::optional<tracer_core::application::ports::LatestActivityRecord> {
  DBManager db_manager(db_path_);
  sqlite3* db_connection = OpenReadDatabaseOrReturnNull(db_manager);
  if (db_connection == nullptr) {
    return std::nullopt;
  }

  const std::string sql = std::format(
      "SELECT {0}, {1}, {2}, {3}, \"{4}\", {5}, {6} FROM {7} "
      "WHERE {0} = ?1 ORDER BY {6} DESC, {8} DESC LIMIT 1;",
      schema::time_records::db::kDate,
      schema::time_records::db::kProjectPathSnapshot,
      schema::time_records::db::kRecordKind,
      schema::time_records::db::kStart,
      schema::time_records::db::kEnd,
      schema::time_records::db::kDuration,
      schema::time_records::db::kEndTimestamp,
      schema::time_records::db::kTable,
      schema::time_records::db::kLogicalId);
  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(db_connection, sql.c_str(), -1, &statement, nullptr) !=
      SQLITE_OK) {
    return std::nullopt;
  }
  if (sqlite3_bind_text(statement, 1, date.data(), static_cast<int>(date.size()),
                        SQLITE_TRANSIENT) != SQLITE_OK) {
    sqlite3_finalize(statement);
    return std::nullopt;
  }

  std::optional<tracer_core::application::ports::LatestActivityRecord> result;
  if (sqlite3_step(statement) == SQLITE_ROW) {
    const std::string result_date = ReadText(statement, 0);
    const std::string activity = ReadText(statement, 1);
    if (!result_date.empty() && !activity.empty()) {
      result = tracer_core::application::ports::LatestActivityRecord{
          .date = result_date,
          .activity = activity,
          .record_kind = ReadText(statement, 2),
          .start_time = ReadText(statement, 3),
          .end_time = ReadText(statement, 4),
          .duration_seconds = sqlite3_column_int(statement, 5),
      };
    }
  }
  sqlite3_finalize(statement);
  return result;
}

}  // namespace tracer::core::infrastructure::persistence
