#include "infra/persistence/importer/repository_ingest_sync_sql.hpp"

#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "infra/schema/sqlite_schema.hpp"

namespace tracer::core::infrastructure::persistence::importer::detail {
namespace {

using tracer_core::core::dto::IngestSyncStatusEntry;

void ThrowIfPrepareFailed(sqlite3* sqlite_db, sqlite3_stmt** statement,
                          const std::string& sql, std::string_view context) {
  if (sqlite3_prepare_v2(sqlite_db, sql.c_str(), -1, statement, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error(std::string(context) + ": " +
                             sqlite3_errmsg(sqlite_db));
  }
}

void BindText(sqlite3_stmt* statement, const int kIndex,
              const std::string& value, std::string_view context) {
  if (sqlite3_bind_text(statement, kIndex, value.c_str(), -1,
                        SQLITE_TRANSIENT) != SQLITE_OK) {
    throw std::runtime_error(std::string(context));
  }
}

void BindInt64(sqlite3_stmt* statement, const int kIndex,
               const std::int64_t kValue, std::string_view context) {
  if (sqlite3_bind_int64(statement, kIndex, kValue) != SQLITE_OK) {
    throw std::runtime_error(std::string(context));
  }
}

void ExecuteStatement(sqlite3* sqlite_db, sqlite3_stmt* statement,
                      std::string_view context) {
  if (sqlite3_step(statement) != SQLITE_DONE) {
    const std::string kMessage =
        std::string(context) + ": " + sqlite3_errmsg(sqlite_db);
    sqlite3_reset(statement);
    sqlite3_clear_bindings(statement);
    throw std::runtime_error(kMessage);
  }
  sqlite3_reset(statement);
  sqlite3_clear_bindings(statement);
}

}  // namespace

auto BuildMonthBoundary(int year, int month) -> std::optional<MonthBoundary> {
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

auto UpsertIngestSyncStatusRow(sqlite3* sqlite_db,
                               const IngestSyncStatusEntry& entry) -> void {
  sqlite3_stmt* statement = nullptr;
  const std::string kSql = std::format(
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
  ThrowIfPrepareFailed(sqlite_db, &statement, kSql,
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

}  // namespace tracer::core::infrastructure::persistence::importer::detail
