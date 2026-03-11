module;

#include "domain/types/ingest_mode.hpp"

export module tracer.core.domain.types.ingest_mode;

export namespace tracer::core::domain::types {

#include "domain/detail/ingest_mode_contract.inc"

}  // namespace tracer::core::domain::types

export namespace tracer::core::domain::modtypes {

using tracer::core::domain::types::IngestMode;

}  // namespace tracer::core::domain::modtypes
