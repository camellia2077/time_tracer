module;

#include <string>
#include <vector>

#include "domain/repositories/i_project_repository.hpp"

export module tracer.core.infrastructure.persistence.runtime.sqlite_project_repository;

export namespace tracer::core::infrastructure::persistence {

#include "infrastructure/persistence/repositories/detail/sqlite_project_repository_decl.inc"

}  // namespace tracer::core::infrastructure::persistence
