// infrastructure/persistence/importer/sqlite/statement.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_STATEMENT_H_
#define INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_STATEMENT_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.persistence.write.importer.sqlite.statement;
#else
#include "infrastructure/sqlite_fwd.hpp"

namespace tracer::core::infrastructure::persistence::importer::sqlite {

#include "infrastructure/persistence/importer/sqlite/detail/statement_decl.inc"

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite
#endif

namespace infrastructure::persistence::importer::sqlite {

using tracer::core::infrastructure::persistence::importer::sqlite::Statement;

}  // namespace infrastructure::persistence::importer::sqlite

#endif  // INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_STATEMENT_H_

