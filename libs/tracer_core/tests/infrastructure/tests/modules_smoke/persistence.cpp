import tracer.core.infrastructure.persistence.runtime;
import tracer.core.infrastructure.persistence.write;

#include "infrastructure/tests/modules_smoke/support.hpp"

auto RunInfrastructureModulePersistenceSmoke() -> int {
  std::error_code cleanup_error;

  const auto kImportData =
      &tracer::core::infrastructure::persistence::importer::Repository::
          ImportData;
  const auto kReplaceMonthData =
      &tracer::core::infrastructure::persistence::importer::Repository::
          ReplaceMonthData;
  const auto kLatestActivityTail =
      &tracer::core::infrastructure::persistence::importer::Repository::
          TryGetLatestActivityTailBeforeDate;
  const auto kExecuteSql =
      &tracer::core::infrastructure::persistence::importer::sqlite::
          ExecuteSql;
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

    tracer::core::infrastructure::persistence::importer::Repository
        repository(kDbPath.string());
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
        persistence_writer(
            connection.GetDb(), statement.GetInsertDayStmt(),
            statement.GetInsertRecordStmt(), statement.GetInsertProjectStmt());
    (void)repository;
    (void)resolver;
    (void)persistence_writer;
  } catch (...) {
    return 25;
  }

  std::filesystem::remove(kDbPath, cleanup_error);

  const auto kCheckReady =
      &tracer::core::infrastructure::persistence::SqliteDatabaseHealthChecker::
          CheckReady;
  const auto kGetAllProjects =
      &tracer::core::infrastructure::persistence::SqliteProjectRepository::
          GetAllProjects;
  (void)kCheckReady;
  (void)kGetAllProjects;

  const std::filesystem::path kPersistenceRuntimeSmokeDir =
      std::filesystem::path("temp") / "phase10_infra_module_smoke";
  std::filesystem::create_directories(kPersistenceRuntimeSmokeDir);
  const std::filesystem::path kRuntimeDbPath =
      kPersistenceRuntimeSmokeDir / "persistence_runtime.sqlite";
  std::filesystem::remove(kRuntimeDbPath, cleanup_error);

  tracer::core::infrastructure::persistence::SqliteDatabaseHealthChecker
      database_health_checker(kRuntimeDbPath.string());
  if (!database_health_checker.CheckReady().ok) {
    return 26;
  }

  try {
    tracer::core::infrastructure::persistence::importer::sqlite::Connection
        runtime_connection(kRuntimeDbPath.string());
    if (runtime_connection.GetDb() == nullptr) {
      return 27;
    }
    if (!tracer::core::infrastructure::persistence::importer::sqlite::
            ExecuteSql(runtime_connection.GetDb(),
                       "INSERT INTO projects "
                       "(id, name, parent_id, full_path, depth) "
                       "VALUES (1, 'Root', NULL, 'Root', 0);",
                       "seed persistence runtime smoke project")) {
      return 28;
    }

    tracer::core::infrastructure::persistence::SqliteProjectRepository
        project_repository(kRuntimeDbPath.string());
    const auto projects = project_repository.GetAllProjects();
    if (projects.size() != 1U || projects.front().name != "Root") {
      return 29;
    }
  } catch (...) {
    return 30;
  }

  std::filesystem::remove(kRuntimeDbPath, cleanup_error);
  std::filesystem::remove_all(kPersistenceRuntimeSmokeDir, cleanup_error);
  return 0;
}
