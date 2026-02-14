// infrastructure/persistence/sqlite_database_health_checker.cpp
#include "infrastructure/persistence/sqlite_database_health_checker.hpp"

#include <utility>

#include "infrastructure/persistence/sqlite/db_manager.hpp"

namespace infrastructure::persistence {

SqliteDatabaseHealthChecker::SqliteDatabaseHealthChecker(std::string db_path)
    : db_path_(std::move(db_path)) {}

auto SqliteDatabaseHealthChecker::CheckReady()
    -> time_tracer::application::ports::DatabaseHealthCheckResult {
  DBManager db_manager(db_path_);
  if (!db_manager.OpenDatabaseIfNeeded()) {
    return {.ok = false, .message = "Failed to open database at: " + db_path_};
  }

  if (db_manager.GetDbConnection() == nullptr) {
    return {.ok = false, .message = "Database connection is null."};
  }

  return {.ok = true, .message = ""};
}

}  // namespace infrastructure::persistence
