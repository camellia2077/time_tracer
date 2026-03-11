module;

#include <string>

#include "application/ports/i_database_health_checker.hpp"

export module tracer.core.infrastructure.persistence.runtime.sqlite_database_health_checker;

export namespace tracer::core::infrastructure::persistence {

#include "infrastructure/persistence/detail/sqlite_database_health_checker_decl.inc"

}  // namespace tracer::core::infrastructure::persistence
