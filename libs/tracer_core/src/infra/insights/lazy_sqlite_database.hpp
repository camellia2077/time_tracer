#ifndef TRACER_CORE_INFRA_INSIGHTS_LAZY_SQLITE_DATABASE_HPP_
#define TRACER_CORE_INFRA_INSIGHTS_LAZY_SQLITE_DATABASE_HPP_

#include <filesystem>
#include <stdexcept>
#include <string>

#include "infra/persistence/sqlite/db_manager.hpp"

namespace tracer::core::infrastructure::insights::detail {

inline auto EnsureReadableDbConnection(const std::filesystem::path& db_path,
                                       DBManager& db_manager) -> sqlite3* {
  if (!std::filesystem::exists(db_path)) {
    throw std::runtime_error("Insights database is not available: " +
                             db_path.string());
  }
  if (!db_manager.OpenDatabaseIfNeeded()) {
    throw std::runtime_error("Insights database could not be opened: " +
                             db_path.string());
  }

  sqlite3* db_connection = db_manager.GetDbConnection();
  if (db_connection == nullptr) {
    throw std::runtime_error("Insights database connection is null: " +
                             db_path.string());
  }
  return db_connection;
}

}  // namespace tracer::core::infrastructure::insights::detail

#endif  // TRACER_CORE_INFRA_INSIGHTS_LAZY_SQLITE_DATABASE_HPP_
