// infrastructure/persistence/repositories/sqlite_project_repository.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_PROJECT_REPOSITORY_H_
#define INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_PROJECT_REPOSITORY_H_

#include <string>

#include "domain/repositories/i_project_repository.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"

namespace tracer::core::infrastructure::persistence {
class SqliteProjectRepository final : public IProjectRepository {
 public:
  explicit SqliteProjectRepository(std::string db_path);

  auto GetAllProjects() -> std::vector<ProjectEntity> override;

 private:
  std::string db_path_;
};

}  // namespace tracer::core::infrastructure::persistence

namespace infrastructure::persistence {

using tracer::core::infrastructure::persistence::SqliteProjectRepository;

}  // namespace infrastructure::persistence

using SqliteProjectRepository =
    tracer::core::infrastructure::persistence::SqliteProjectRepository;

#endif  // INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_PROJECT_REPOSITORY_H_

