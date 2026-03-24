import tracer.core.infrastructure.persistence.runtime;
import tracer.core.infrastructure.persistence.write;

#include "infra/tests/modules_smoke/persistence_runtime.hpp"

#include <filesystem>

namespace {

auto RunPersistenceRuntimeSmokeImpl() -> int {
  std::error_code cleanup_error;

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

}  // namespace

auto RunInfrastructureModulePersistenceRuntimeSmoke() -> int {
  return RunPersistenceRuntimeSmokeImpl();
}
