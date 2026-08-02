import tracer.core.infrastructure.persistence.write;

#include "infra/tests/modules_smoke/persistence_write.hpp"
#include "application/pipeline/importer/model/import_models.hpp"

#include <sqlite3.h>

#include <filesystem>
#include <optional>
#include <string_view>

namespace {

auto RunPersistenceWriteSmokeImpl() -> int {
  std::error_code cleanup_error;

  const auto kImportData = &tracer::core::infrastructure::persistence::
                               importer::Repository::ImportData;
  const auto kReplaceMonthData = &tracer::core::infrastructure::persistence::
                                     importer::Repository::ReplaceMonthData;
  const auto kLatestActivityTail =
      &tracer::core::infrastructure::persistence::importer::Repository::
          TryGetLatestActivityTailBeforeDate;
  const auto kExecuteSql =
      &tracer::core::infrastructure::persistence::importer::sqlite::ExecuteSql;
  (void)kImportData;
  (void)kReplaceMonthData;
  (void)kLatestActivityTail;
  (void)kExecuteSql;

  const std::filesystem::path kPersistenceSmokeDir =
      std::filesystem::path("temp") / "phase7_infra_module_smoke";
  std::filesystem::create_directories(kPersistenceSmokeDir);
  const std::filesystem::path kDbPath =
      kPersistenceSmokeDir / "persistence_write.sqlite";
  std::filesystem::remove(kDbPath, cleanup_error);

  try {
    tracer::core::infrastructure::persistence::SqliteTimeSheetRepository
        time_sheet_repository(kDbPath.string());
    if (time_sheet_repository.IsDbOpen()) {
      return 22;
    }

    tracer::core::infrastructure::persistence::importer::Repository repository(
        kDbPath.string());
    tracer::core::infrastructure::persistence::importer::sqlite::Connection
        connection(kDbPath.string());
    if (connection.GetDb() == nullptr) {
      return 23;
    }

    tracer::core::infrastructure::persistence::importer::sqlite::Statement
        statement(connection.GetDb());
    if (statement.GetInsertDayStmt() == nullptr ||
        statement.GetInsertRecordStmt() == nullptr ||
        statement.GetInsertProjectStmt() == nullptr) {
      return 24;
    }

    tracer::core::infrastructure::persistence::importer::sqlite::ProjectResolver
        resolver(connection.GetDb(), statement.GetInsertProjectStmt());
    tracer::core::infrastructure::persistence::importer::sqlite::Writer
        persistence_writer(connection.GetDb(), statement.GetInsertDayStmt(),
                           statement.GetInsertRecordStmt(),
                           statement.GetInsertProjectStmt());
    (void)repository;
    (void)resolver;

    persistence_writer.InsertDays({DayData{.date = "2026-02-01",
                                           .remark = "",
                                           .getup_time = std::nullopt,
                                           .year = 2026,
                                           .month = 2,
                                           .activity_count = 1}});
    persistence_writer.InsertRecords(
        {TimeRecordInternal{.kind = ActivityRecordKind::kEndOnly,
                             .logical_id = 1,
                             .start_timestamp = 0,
                             .end_timestamp = 1770000000,
                             .start_time_str = "",
                             .end_time_str = "12:00",
                             .project_path = "work",
                             .duration_seconds = 0,
                             .remark = std::nullopt,
                             .date = "2026-02-01"}});

    sqlite3_stmt* record_stmt = nullptr;
    if (sqlite3_prepare_v2(
            connection.GetDb(),
            "SELECT record_kind, start, duration FROM time_records WHERE "
            "logical_id = 1;",
            -1, &record_stmt, nullptr) != SQLITE_OK ||
        sqlite3_step(record_stmt) != SQLITE_ROW) {
      sqlite3_finalize(record_stmt);
      return 26;
    }
    const auto* kind_text = sqlite3_column_text(record_stmt, 0);
    const auto* start_text = sqlite3_column_text(record_stmt, 1);
    const int duration = sqlite3_column_int(record_stmt, 2);
    const bool persisted_end_only =
        kind_text != nullptr && start_text != nullptr &&
        std::string_view(reinterpret_cast<const char*>(kind_text)) ==
            "end_only" &&
        std::string_view(reinterpret_cast<const char*>(start_text)).empty() &&
        duration == 0;
    sqlite3_finalize(record_stmt);
    if (!persisted_end_only) {
      return 27;
    }
  } catch (...) {
    return 25;
  }

  std::filesystem::remove(kDbPath, cleanup_error);
  std::filesystem::remove_all(kPersistenceSmokeDir, cleanup_error);
  return 0;
}

}  // namespace

auto RunInfrastructureModulePersistenceWriteSmoke() -> int {
  return RunPersistenceWriteSmokeImpl();
}
