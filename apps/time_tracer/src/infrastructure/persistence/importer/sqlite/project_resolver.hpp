// infrastructure/persistence/importer/sqlite/project_resolver.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_PROJECT_RESOLVER_H_
#define INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_PROJECT_RESOLVER_H_

#include <sqlite3.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace infrastructure::persistence::importer::sqlite {

struct ImportProjectNode;

class ProjectResolver {
 public:
  explicit ProjectResolver(sqlite3* db_ptr, sqlite3_stmt* stmt_insert_project);
  ~ProjectResolver();

  auto PreloadAndResolve(const std::vector<std::string>& project_paths) -> void;

  [[nodiscard]] auto GetId(const std::string& project_path) const -> long long;

 private:
  sqlite3* db_;
  sqlite3_stmt* stmt_insert_project_;

  std::unique_ptr<ImportProjectNode> root_;
  std::unordered_map<std::string, long long> cache_;

  auto LoadFromDb() -> void;
  [[nodiscard]] auto EnsurePath(const std::string& path) -> long long;
};

}  // namespace infrastructure::persistence::importer::sqlite

#endif  // INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_PROJECT_RESOLVER_H_
