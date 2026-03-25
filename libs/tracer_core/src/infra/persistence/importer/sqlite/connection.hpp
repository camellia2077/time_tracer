// infra/persistence/importer/sqlite/connection.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_CONNECTION_H_
#define INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_CONNECTION_H_

#include "infra/sqlite_fwd.hpp"

#include <string>

namespace tracer::core::infrastructure::persistence::importer::sqlite {
class Connection {
 public:
  explicit Connection(const std::string& db_path);
  ~Connection();

  [[nodiscard]] auto GetDb() const -> sqlite3*;
  auto BeginTransaction() -> bool;
  auto CommitTransaction() -> bool;
  auto RollbackTransaction() -> void;

 private:
  sqlite3* db_{nullptr};
};

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto ExecuteSql(sqlite3* sqlite_db, const std::string& sql_query,
                const std::string& error_context = "") -> bool;
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite

namespace infrastructure::persistence::importer::sqlite {

using tracer::core::infrastructure::persistence::importer::sqlite::Connection;
using tracer::core::infrastructure::persistence::importer::sqlite::ExecuteSql;

}  // namespace infrastructure::persistence::importer::sqlite

#endif  // INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_CONNECTION_H_
