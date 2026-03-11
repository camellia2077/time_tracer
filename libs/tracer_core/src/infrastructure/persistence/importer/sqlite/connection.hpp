// infrastructure/persistence/importer/sqlite/connection.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_CONNECTION_H_
#define INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_CONNECTION_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.persistence.write.importer.sqlite.connection;
#else
#include "infrastructure/sqlite_fwd.hpp"

#include <string>

namespace tracer::core::infrastructure::persistence::importer::sqlite {

#include "infrastructure/persistence/importer/sqlite/detail/connection_decl.inc"

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite
#endif

namespace infrastructure::persistence::importer::sqlite {

using tracer::core::infrastructure::persistence::importer::sqlite::Connection;
using tracer::core::infrastructure::persistence::importer::sqlite::ExecuteSql;

}  // namespace infrastructure::persistence::importer::sqlite

#endif  // INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_CONNECTION_H_

