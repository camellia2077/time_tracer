import tracer.core.infrastructure.persistence.write;

#include "infra/tests/modules_smoke/persistence_write.hpp"

#include <filesystem>

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
    (void)persistence_writer;
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
