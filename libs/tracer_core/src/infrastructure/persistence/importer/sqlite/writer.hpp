// infrastructure/persistence/importer/sqlite/writer.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_WRITER_H_
#define INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_WRITER_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.persistence.write.importer.sqlite.writer;
#else
#include "infrastructure/sqlite_fwd.hpp"

#include <memory>
#include <string>
#include <vector>

#include "application/importer/model/import_models.hpp"

namespace tracer::core::infrastructure::persistence::importer::sqlite {

#include "infrastructure/persistence/importer/sqlite/detail/writer_decl.inc"

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite
#endif

namespace infrastructure::persistence::importer::sqlite {

using tracer::core::infrastructure::persistence::importer::sqlite::Writer;

}  // namespace infrastructure::persistence::importer::sqlite

#endif  // INFRASTRUCTURE_PERSISTENCE_IMPORTER_SQLITE_WRITER_H_

