// importer/storage/sqlite/project_resolver.hpp
#ifndef IMPORTER_STORAGE_SQLITE_PROJECT_RESOLVER_H_
#define IMPORTER_STORAGE_SQLITE_PROJECT_RESOLVER_H_

#include <sqlite3.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct ImportProjectNode;  // Forward declaration

class ProjectResolver {
 public:
  explicit ProjectResolver(sqlite3* db_ptr, sqlite3_stmt* stmt_insert_project);
  ~ProjectResolver();

  // Batch preload and resolve
  void PreloadAndResolve(const std::vector<std::string>& project_paths);

  // Get ID from cache
  [[nodiscard]] auto GetId(const std::string& project_path) const -> long long;

 private:
  sqlite3* db_;
  sqlite3_stmt* stmt_insert_project_;

  std::unique_ptr<ImportProjectNode> root_;
  std::unordered_map<std::string, long long> cache_;

  void LoadFromDb();
  [[nodiscard]] auto EnsurePath(const std::string& path) -> long long;
};

#endif  // IMPORTER_STORAGE_SQLITE_PROJECT_RESOLVER_H_