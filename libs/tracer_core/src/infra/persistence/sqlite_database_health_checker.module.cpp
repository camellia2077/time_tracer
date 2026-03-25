#include <filesystem>
#include <string>
#include <utility>

#include "infra/persistence/sqlite_database_health_checker.hpp"
#include "application/ports/pipeline/i_database_health_checker.hpp"

namespace tracer::core::infrastructure::persistence {

SqliteDatabaseHealthChecker::SqliteDatabaseHealthChecker(std::string db_path)
    : db_path_(std::move(db_path)) {}

auto SqliteDatabaseHealthChecker::CheckReady()
    -> tracer_core::application::ports::DatabaseHealthCheckResult {
  const std::filesystem::path kDbPath(db_path_);
  if (kDbPath.empty()) {
    return {.ok = false, .message = "Database path must not be empty."};
  }

  const std::filesystem::path kParentPath = kDbPath.parent_path();
  if (kParentPath.empty()) {
    return {.ok = false,
            .message = "Database parent path must not be empty: " + db_path_};
  }

  if (std::filesystem::exists(kParentPath) &&
      !std::filesystem::is_directory(kParentPath)) {
    return {.ok = false,
            .message = "Database parent path is not a directory: " +
                       kParentPath.string()};
  }

  return {.ok = true, .message = ""};
}

}  // namespace tracer::core::infrastructure::persistence
