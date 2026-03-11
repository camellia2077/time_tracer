// infrastructure/persistence/sqlite_database_health_checker.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_SQLITE_DATABASE_HEALTH_CHECKER_H_
#define INFRASTRUCTURE_PERSISTENCE_SQLITE_DATABASE_HEALTH_CHECKER_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.persistence.runtime.sqlite_database_health_checker;
#else
#include <string>

#include "application/ports/i_database_health_checker.hpp"

namespace tracer::core::infrastructure::persistence {

#include "infrastructure/persistence/detail/sqlite_database_health_checker_decl.inc"

}  // namespace tracer::core::infrastructure::persistence
#endif

namespace infrastructure::persistence {

using tracer::core::infrastructure::persistence::SqliteDatabaseHealthChecker;

}  // namespace infrastructure::persistence

#endif  // INFRASTRUCTURE_PERSISTENCE_SQLITE_DATABASE_HEALTH_CHECKER_H_
