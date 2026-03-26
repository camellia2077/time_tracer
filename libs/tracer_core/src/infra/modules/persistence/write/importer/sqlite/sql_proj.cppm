module;

#include "infra/sqlite_fwd.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

export module tracer.core.infrastructure.persistence.write.importer.sqlite
    .project_resolver;

export namespace tracer::core::infrastructure::persistence::importer::sqlite {
struct ImportProjectNode;

class ProjectResolver {
 public:
  explicit ProjectResolver(sqlite3* db_ptr, sqlite3_stmt* stmt_insert_project);
  ~ProjectResolver();

  auto PreloadAndResolve(const std::vector<std::string>& project_paths) -> void;

  [[nodiscard]] auto GetId(const std::string& project_path) const
      -> std::int64_t;

 private:
  sqlite3* db_;
  sqlite3_stmt* stmt_insert_project_;

  std::unique_ptr<ImportProjectNode> root_;
  std::unordered_map<std::string, std::int64_t> cache_;

  auto LoadFromDb() -> void;
  [[nodiscard]] auto EnsurePath(const std::string& path) -> std::int64_t;
};

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite
