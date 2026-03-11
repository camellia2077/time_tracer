module;

#include "infrastructure/sqlite_fwd.hpp"

#include <string>

export module tracer.core.infrastructure.persistence.write.importer.sqlite.connection;

export namespace tracer::core::infrastructure::persistence::importer::sqlite {

#include "infrastructure/persistence/importer/sqlite/detail/connection_decl.inc"

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite
