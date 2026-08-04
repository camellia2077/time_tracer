module;

#include "infra/persistence/repositories/sqlite_ingest_runtime_repository.hpp"

export module tracer.core.infrastructure.persistence.runtime
    .sqlite_ingest_runtime_repository;

export namespace tracer::core::infrastructure::persistence {

using ::tracer::core::infrastructure::persistence::
    SqliteIngestRuntimeRepository;

}  // namespace tracer::core::infrastructure::persistence
