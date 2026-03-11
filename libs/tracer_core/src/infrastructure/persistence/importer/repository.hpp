// infrastructure/persistence/importer/repository.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_H_
#define INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_H_

#if TT_ENABLE_CPP20_MODULES && !defined(TT_FORCE_LEGACY_HEADER_DECLS)
import tracer.core.infrastructure.persistence.write.importer.repository;
#else
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "application/importer/model/import_models.hpp"

namespace tracer::core::infrastructure::persistence::importer::sqlite {

class Connection;
class Statement;
class Writer;

}  // namespace tracer::core::infrastructure::persistence::importer::sqlite

namespace tracer::core::infrastructure::persistence::importer {

#include "infrastructure/persistence/importer/detail/repository_decl.inc"

}  // namespace tracer::core::infrastructure::persistence::importer
#endif

namespace infrastructure::persistence::importer {

using tracer::core::infrastructure::persistence::importer::Repository;

}  // namespace infrastructure::persistence::importer

#endif  // INFRASTRUCTURE_PERSISTENCE_IMPORTER_REPOSITORY_H_
