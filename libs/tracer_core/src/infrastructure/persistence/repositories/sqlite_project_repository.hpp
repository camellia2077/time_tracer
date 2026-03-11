// infrastructure/persistence/repositories/sqlite_project_repository.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_PROJECT_REPOSITORY_H_
#define INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_PROJECT_REPOSITORY_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.persistence.runtime.sqlite_project_repository;
#else
#include <string>

#include "domain/repositories/i_project_repository.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"

namespace tracer::core::infrastructure::persistence {

#include "infrastructure/persistence/repositories/detail/sqlite_project_repository_decl.inc"

}  // namespace tracer::core::infrastructure::persistence
#endif

namespace infrastructure::persistence {

using tracer::core::infrastructure::persistence::SqliteProjectRepository;

}  // namespace infrastructure::persistence

using SqliteProjectRepository =
    tracer::core::infrastructure::persistence::SqliteProjectRepository;

#endif  // INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_PROJECT_REPOSITORY_H_

