// domain/repositories/i_project_repository.hpp
#ifndef DOMAIN_REPOSITORIES_I_PROJECT_REPOSITORY_H_
#define DOMAIN_REPOSITORIES_I_PROJECT_REPOSITORY_H_

#include <optional>
#include <string>
#include <vector>

struct ProjectEntity {
  int id;
  std::optional<int> parent_id;
  std::string name;
};

class IProjectRepository {
 public:
  virtual ~IProjectRepository() = default;

  virtual auto GetAllProjects() -> std::vector<ProjectEntity> = 0;
};

#endif  // DOMAIN_REPOSITORIES_I_PROJECT_REPOSITORY_H_
