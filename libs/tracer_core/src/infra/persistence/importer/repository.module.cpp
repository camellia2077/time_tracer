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
#include "infra/persistence/importer/repository_ingest_sync_sql.hpp"
#include "application/pipeline/importer/model/import_models.hpp"
#include "infra/schema/day_schema.hpp"
#include "infra/schema/sqlite_schema.hpp"

import tracer.core.infrastructure.persistence.write.importer.sqlite;

namespace tracer::core::infrastructure::persistence::importer {

using tracer_core::core::dto::IngestSyncStatusEntry;

namespace {}  // namespace

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

  const auto kBoundary = detail::BuildMonthBoundary(kYear, kMonth);
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

auto Repository::UpsertIngestSyncStatus(const IngestSyncStatusEntry& entry)
    -> void {
  EnsureWriteRepositoryReady();
  detail::UpsertIngestSyncStatusRow(connection_manager_->GetDb(), entry);
}

auto Repository::ReplaceIngestSyncStatuses(
    const std::vector<IngestSyncStatusEntry>& entries) -> void {
  EnsureWriteRepositoryReady();

  if (!connection_manager_->BeginTransaction()) {
    throw std::runtime_error(
        "Failed to begin ingest sync replacement transaction.");
  }

  try {
    const std::string delete_sql =
        std::format("DELETE FROM {0};", schema::ingest_month_sync::db::kTable);
    if (!sqlite::ExecuteSql(connection_manager_->GetDb(), delete_sql,
                            "Delete ingest month sync rows")) {
      throw std::runtime_error("Failed to delete ingest sync statuses.");
    }

    for (const auto& entry : entries) {
      detail::UpsertIngestSyncStatusRow(connection_manager_->GetDb(), entry);
    }

    if (!connection_manager_->CommitTransaction()) {
      throw std::runtime_error(
          "Failed to commit ingest sync replacement transaction.");
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

}  // namespace tracer::core::infrastructure::persistence::importer
