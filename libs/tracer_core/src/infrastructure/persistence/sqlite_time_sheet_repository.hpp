// infrastructure/persistence/sqlite_time_sheet_repository.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_SQLITE_TIME_SHEET_REPOSITORY_H_
#define INFRASTRUCTURE_PERSISTENCE_SQLITE_TIME_SHEET_REPOSITORY_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.persistence.write.sqlite_time_sheet_repository;
#else
#include <optional>
#include <string>
#include <string_view>

#include "application/ports/i_time_sheet_repository.hpp"
#include "infrastructure/persistence/importer/repository.hpp"

namespace tracer::core::infrastructure::persistence {

#include "infrastructure/persistence/detail/sqlite_time_sheet_repository_decl.inc"

}  // namespace tracer::core::infrastructure::persistence
#endif

namespace infrastructure::persistence {

using tracer::core::infrastructure::persistence::SqliteTimeSheetRepository;

}  // namespace infrastructure::persistence

#endif  // INFRASTRUCTURE_PERSISTENCE_SQLITE_TIME_SHEET_REPOSITORY_H_
