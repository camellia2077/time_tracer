#include <sqlite3.h>

#include <filesystem>
#include <format>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "infra/persistence/importer/repository.hpp"
#include "application/pipeline/importer/model/import_models.hpp"
#include "infra/persistence/sqlite/db_manager.hpp"
#include "infra/schema/day_schema.hpp"
#include "infra/schema/sqlite_schema.hpp"

import tracer.core.infrastructure.persistence.write.importer.sqlite;

namespace tracer::core::infrastructure::persistence::importer {

namespace {

struct MonthBoundary {
  std::string start_date;
  std::string next_month_start_date;
};

using tracer_core::core::dto::IngestSyncStatusEntry;
using tracer_core::core::dto::IngestSyncStatusOutput;
using tracer_core::core::dto::IngestSyncStatusRequest;

[[nodiscard]] auto BuildMonthBoundary(int year, int month)
    -> std::optional<MonthBoundary> {
  constexpr int kMonthsPerYear = 12;
  if (month < 1 || month > kMonthsPerYear) {
    return std::nullopt;
  }

  int next_year = year;
  int next_month = month + 1;
  if (next_month > kMonthsPerYear) {
    next_month = 1;
    ++next_year;
  }

  return MonthBoundary{
      .start_date = std::format("{:04d}-{:02d}-01", year, month),
      .next_month_start_date =
          std::format("{:04d}-{:02d}-01", next_year, next_month)};
}

void ThrowIfPrepareFailed(sqlite3* sqlite_db, sqlite3_stmt** statement,
                          const std::string& sql,
                          std::string_view context) {
  if (sqlite3_prepare_v2(sqlite_db, sql.c_str(), -1, statement, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error(std::string(context) + ": " +
                             sqlite3_errmsg(sqlite_db));
  }
}

void BindText(sqlite3_stmt* statement, const int index,
              const std::string& value, std::string_view context) {
  if (sqlite3_bind_text(statement, index, value.c_str(), -1,
                        SQLITE_TRANSIENT) != SQLITE_OK) {
    throw std::runtime_error(std::string(context));
  }
}

void BindInt64(sqlite3_stmt* statement, const int index, const std::int64_t value,
               std::string_view context) {
  if (sqlite3_bind_int64(statement, index, value) != SQLITE_OK) {
    throw std::runtime_error(std::string(context));
  }
}

void ExecuteStatement(sqlite3* sqlite_db, sqlite3_stmt* statement,
                      std::string_view context) {
  if (sqlite3_step(statement) != SQLITE_DONE) {
    const std::string message =
        std::string(context) + ": " + sqlite3_errmsg(sqlite_db);
    sqlite3_reset(statement);
    sqlite3_clear_bindings(statement);
    throw std::runtime_error(message);
  }
  sqlite3_reset(statement);
  sqlite3_clear_bindings(statement);
}

void UpsertIngestSyncStatusRow(sqlite3* sqlite_db,
                               const IngestSyncStatusEntry& entry) {
  sqlite3_stmt* statement = nullptr;
  const std::string sql = std::format(
      "INSERT INTO {0} ({1}, {2}, {3}, {4}) VALUES (?1, ?2, ?3, ?4) "
      "ON CONFLICT({1}) DO UPDATE SET "
      "{2}=excluded.{2}, "
      "{3}=excluded.{3}, "
      "{4}=excluded.{4};",
      schema::ingest_month_sync::db::kTable,
      schema::ingest_month_sync::db::kMonthKey,
      schema::ingest_month_sync::db::kTxtRelativePath,
      schema::ingest_month_sync::db::kTxtContentHashSha256,
      schema::ingest_month_sync::db::kIngestedAtUnixMs);
  ThrowIfPrepareFailed(sqlite_db, &statement, sql,
                       "Prepare ingest sync upsert statement failed");

  try {
    BindText(statement, 1, entry.month_key, "Bind month_key failed");
    BindText(statement, 2, entry.txt_relative_path,
             "Bind txt_relative_path failed");
    BindText(statement, 3, entry.txt_content_hash_sha256,
             "Bind txt_content_hash_sha256 failed");
    BindInt64(statement, 4, entry.ingested_at_unix_ms,
              "Bind ingested_at_unix_ms failed");
    ExecuteStatement(sqlite_db, statement, "Execute ingest sync upsert failed");
  } catch (...) {
    sqlite3_finalize(statement);
    throw;
  }
  sqlite3_finalize(statement);
}

auto ListIngestSyncStatusRows(sqlite3* sqlite_db,
                              const IngestSyncStatusRequest& request)
    -> IngestSyncStatusOutput {
  std::string sql = std::format(
      "SELECT {1}, {2}, {3}, {4} FROM {0}",
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
      sql += std::format("{0} = ?{1}",
                         schema::ingest_month_sync::db::kMonthKey, index + 1U);
    }
  }
  sql += std::format(" ORDER BY {0} ASC;",
                     schema::ingest_month_sync::db::kMonthKey);

  sqlite3_stmt* statement = nullptr;
  ThrowIfPrepareFailed(sqlite_db, &statement, sql,
                       "Prepare ingest sync query failed");

  try {
    for (size_t index = 0; index < request.months.size(); ++index) {
      BindText(statement, static_cast<int>(index + 1U), request.months[index],
               "Bind ingest sync query month failed");
    }

    IngestSyncStatusOutput output{.ok = true, .items = {}, .error_message = ""};
    for (;;) {
      const int step = sqlite3_step(statement);
      if (step == SQLITE_DONE) {
        break;
      }
      if (step != SQLITE_ROW) {
        throw std::runtime_error("Execute ingest sync query failed: " +
                                 std::string(sqlite3_errmsg(sqlite_db)));
      }

      const auto* month_key = sqlite3_column_text(statement, 0);
      const auto* txt_relative_path = sqlite3_column_text(statement, 1);
      const auto* txt_content_hash_sha256 = sqlite3_column_text(statement, 2);
      output.items.push_back(IngestSyncStatusEntry{
          .month_key = month_key == nullptr
                           ? std::string{}
                           : reinterpret_cast<const char*>(month_key),
          .txt_relative_path =
              txt_relative_path == nullptr
                  ? std::string{}
                  : reinterpret_cast<const char*>(txt_relative_path),
          .txt_content_hash_sha256 =
              txt_content_hash_sha256 == nullptr
                  ? std::string{}
                  : reinterpret_cast<const char*>(txt_content_hash_sha256),
          .ingested_at_unix_ms = static_cast<std::int64_t>(
              sqlite3_column_int64(statement, 3)),
      });
    }

    sqlite3_finalize(statement);
    return output;
  } catch (...) {
    sqlite3_finalize(statement);
    throw;
  }
}

}  // namespace

Repository::Repository(std::string db_path) : db_path_(std::move(db_path)) {}

Repository::~Repository() = default;

auto Repository::IsDbOpen() const -> bool {
  return connection_manager_ && (connection_manager_->GetDb() != nullptr);
}

auto Repository::EnsureWriteRepositoryReady() -> void {
  if (IsDbOpen() && statement_manager_ && data_inserter_) {
    return;
  }

  const std::filesystem::path kDbPath(db_path_);
  if (!kDbPath.parent_path().empty()) {
    std::filesystem::create_directories(kDbPath.parent_path());
  }

  connection_manager_ = std::make_unique<sqlite::Connection>(db_path_);
  if (!connection_manager_ || connection_manager_->GetDb() == nullptr) {
    throw std::runtime_error("Failed to open write database: " + db_path_);
  }

  statement_manager_ =
      std::make_unique<sqlite::Statement>(connection_manager_->GetDb());
  data_inserter_ = std::make_unique<sqlite::Writer>(
      connection_manager_->GetDb(), statement_manager_->GetInsertDayStmt(),
      statement_manager_->GetInsertRecordStmt(),
      statement_manager_->GetInsertProjectStmt());
}

auto Repository::ImportData(const std::vector<DayData>& days,
                            const std::vector<TimeRecordInternal>& records)
    -> void {
  EnsureWriteRepositoryReady();

  if (!connection_manager_->BeginTransaction()) {
    throw std::runtime_error("Failed to begin transaction.");
  }

  try {
    data_inserter_->InsertDays(days);
    data_inserter_->InsertRecords(records);

    if (!connection_manager_->CommitTransaction()) {
      throw std::runtime_error("Failed to commit transaction.");
    }
  } catch (const std::exception&) {
    connection_manager_->RollbackTransaction();
    throw;
  }
}

auto Repository::ReplaceAllData(const std::vector<DayData>& days,
                                const std::vector<TimeRecordInternal>& records)
    -> void {
  EnsureWriteRepositoryReady();

  if (!connection_manager_->BeginTransaction()) {
    throw std::runtime_error("Failed to begin transaction.");
  }

  try {
    const std::string delete_records_sql =
        std::format("DELETE FROM {0};", schema::time_records::db::kTable);
    if (!sqlite::ExecuteSql(connection_manager_->GetDb(), delete_records_sql,
                            "Delete all rows from time_records")) {
      throw std::runtime_error("Failed to delete all data from time_records.");
    }

    const std::string delete_days_sql =
        std::format("DELETE FROM {0};", schema::day::db::kTable);
    if (!sqlite::ExecuteSql(connection_manager_->GetDb(), delete_days_sql,
                            "Delete all rows from days")) {
      throw std::runtime_error("Failed to delete all data from days.");
    }

    const std::string delete_projects_sql =
        std::format("DELETE FROM {0};", schema::projects::db::kTable);
    if (!sqlite::ExecuteSql(connection_manager_->GetDb(), delete_projects_sql,
                            "Delete all rows from projects")) {
      throw std::runtime_error("Failed to delete all data from projects.");
    }

    data_inserter_->InsertDays(days);
    data_inserter_->InsertRecords(records);

    if (!connection_manager_->CommitTransaction()) {
      throw std::runtime_error("Failed to commit transaction.");
    }
  } catch (const std::exception&) {
    connection_manager_->RollbackTransaction();
    throw;
  }
}

auto Repository::ReplaceMonthData(
    const int kYear, const int kMonth, const std::vector<DayData>& days,
    const std::vector<TimeRecordInternal>& records) -> void {
  EnsureWriteRepositoryReady();

  const auto kBoundary = BuildMonthBoundary(kYear, kMonth);
  if (!kBoundary.has_value()) {
    throw std::runtime_error("Invalid replace-month target.");
  }

  if (!connection_manager_->BeginTransaction()) {
    throw std::runtime_error("Failed to begin transaction.");
  }

  try {
    const std::string kDeleteRecordsSql = std::format(
        "DELETE FROM {0} WHERE {1} >= '{2}' AND {1} < '{3}';",
        schema::time_records::db::kTable, schema::time_records::db::kDate,
        kBoundary->start_date, kBoundary->next_month_start_date);
    if (!sqlite::ExecuteSql(connection_manager_->GetDb(), kDeleteRecordsSql,
                            "Delete month rows from time_records")) {
      throw std::runtime_error(
          "Failed to delete month data from time_records.");
    }

    const std::string kDeleteDaysSql =
        std::format("DELETE FROM {0} WHERE {1} >= '{2}' AND {1} < '{3}';",
                    schema::day::db::kTable, schema::day::db::kDate,
                    kBoundary->start_date, kBoundary->next_month_start_date);
    if (!sqlite::ExecuteSql(connection_manager_->GetDb(), kDeleteDaysSql,
                            "Delete month rows from days")) {
      throw std::runtime_error("Failed to delete month data from days.");
    }

    data_inserter_->InsertDays(days);
    data_inserter_->InsertRecords(records);

    if (!connection_manager_->CommitTransaction()) {
      throw std::runtime_error("Failed to commit transaction.");
    }
  } catch (const std::exception&) {
    connection_manager_->RollbackTransaction();
    throw;
  }
}

auto Repository::UpsertIngestSyncStatus(
    const IngestSyncStatusEntry& entry) -> void {
  EnsureWriteRepositoryReady();
  UpsertIngestSyncStatusRow(connection_manager_->GetDb(), entry);
}

auto Repository::ReplaceIngestSyncStatuses(
    const std::vector<IngestSyncStatusEntry>& entries) -> void {
  EnsureWriteRepositoryReady();

  if (!connection_manager_->BeginTransaction()) {
    throw std::runtime_error("Failed to begin ingest sync replacement transaction.");
  }

  try {
    const std::string delete_sql =
        std::format("DELETE FROM {0};", schema::ingest_month_sync::db::kTable);
    if (!sqlite::ExecuteSql(connection_manager_->GetDb(), delete_sql,
                            "Delete ingest month sync rows")) {
      throw std::runtime_error("Failed to delete ingest sync statuses.");
    }

    for (const auto& entry : entries) {
      UpsertIngestSyncStatusRow(connection_manager_->GetDb(), entry);
    }

    if (!connection_manager_->CommitTransaction()) {
      throw std::runtime_error("Failed to commit ingest sync replacement transaction.");
    }
  } catch (const std::exception&) {
    connection_manager_->RollbackTransaction();
    throw;
  }
}

auto Repository::ClearIngestSyncStatus() -> void {
  EnsureWriteRepositoryReady();
  const std::string delete_sql =
      std::format("DELETE FROM {0};", schema::ingest_month_sync::db::kTable);
  if (!sqlite::ExecuteSql(connection_manager_->GetDb(), delete_sql,
                          "Delete ingest month sync rows")) {
    throw std::runtime_error("Failed to clear ingest sync statuses.");
  }
}

auto Repository::ListIngestSyncStatuses(const IngestSyncStatusRequest& request)
    const -> IngestSyncStatusOutput {
  sqlite3* db_connection = nullptr;
  DBManager db_manager(db_path_);

  if (IsDbOpen()) {
    db_connection = connection_manager_->GetDb();
  } else {
    if (!db_manager.OpenDatabaseIfNeeded()) {
      return {.ok = true, .items = {}, .error_message = ""};
    }
    db_connection = db_manager.GetDbConnection();
  }

  if (db_connection == nullptr) {
    return {.ok = true, .items = {}, .error_message = ""};
  }

  return ListIngestSyncStatusRows(db_connection, request);
}

auto Repository::TryGetLatestActivityTailBeforeDate(std::string_view date) const
    -> std::optional<LatestActivityTail> {
  sqlite3* db_connection = nullptr;
  DBManager db_manager(db_path_);

  if (IsDbOpen()) {
    db_connection = connection_manager_->GetDb();
  } else {
    if (!db_manager.OpenDatabaseIfNeeded()) {
      return std::nullopt;
    }
    db_connection = db_manager.GetDbConnection();
  }

  if (db_connection == nullptr) {
    return std::nullopt;
  }

  const std::string kSql = std::format(
      "SELECT {0}, \"{1}\" FROM {2} WHERE {0} < ?1 "
      "ORDER BY {0} DESC, {3} DESC LIMIT 1;",
      schema::time_records::db::kDate, schema::time_records::db::kEnd,
      schema::time_records::db::kTable,
      schema::time_records::db::kEndTimestamp);

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_connection, kSql.c_str(), -1, &stmt, nullptr) !=
      SQLITE_OK) {
    return std::nullopt;
  }

  sqlite3_bind_text(stmt, 1, date.data(), static_cast<int>(date.size()),
                    SQLITE_TRANSIENT);

  std::optional<LatestActivityTail> result;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* date_text = sqlite3_column_text(stmt, 0);
    const unsigned char* end_text = sqlite3_column_text(stmt, 1);
    if (date_text != nullptr && end_text != nullptr) {
      result = LatestActivityTail{
          .date = reinterpret_cast<const char*>(date_text),
          .end_time = reinterpret_cast<const char*>(end_text)};
    }
  }
  sqlite3_finalize(stmt);
  return result;
}

}  // namespace tracer::core::infrastructure::persistence::importer
