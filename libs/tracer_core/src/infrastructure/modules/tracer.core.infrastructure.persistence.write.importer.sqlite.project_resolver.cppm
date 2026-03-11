module;

#include "infrastructure/sqlite_fwd.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

export module tracer.core.infrastructure.persistence.write.importer.sqlite.project_resolver;

export namespace tracer::core::infrastructure::persistence::importer::sqlite {

#include "infrastructure/persistence/importer/sqlite/detail/project_resolver_decl.inc"

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite
