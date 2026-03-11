// infrastructure/persistence/importer/sqlite/project_resolver.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_PROJECT_RESOLVER_H_
#define INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_PROJECT_RESOLVER_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.persistence.write.importer.sqlite.project_resolver;
#else
#include "infrastructure/sqlite_fwd.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace tracer::core::infrastructure::persistence::importer::sqlite {

#include "infrastructure/persistence/importer/sqlite/detail/project_resolver_decl.inc"

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite
#endif

namespace infrastructure::persistence::importer::sqlite {

using tracer::core::infrastructure::persistence::importer::sqlite::ProjectResolver;

}  // namespace infrastructure::persistence::importer::sqlite

#endif  // INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_PROJECT_RESOLVER_H_

