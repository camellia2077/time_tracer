// infrastructure/persistence/repositories/sqlite_project_repository.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_PROJECT_REPOSITORY_H_
#define INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_PROJECT_REPOSITORY_H_

#include <sqlite3.h>

#include <filesystem>

#include "domain/repositories/i_project_repository.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"

class SqliteProjectRepository : public IProjectRepository {
 public:
  explicit SqliteProjectRepository(std::string db_path);

  auto GetAllProjects() -> std::vector<ProjectEntity> override;

 private:
  std::string db_path_;
};

#endif  // INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_PROJECT_REPOSITORY_H_
