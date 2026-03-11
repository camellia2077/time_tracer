// domain/types/ingest_mode.hpp
#ifndef DOMAIN_TYPES_INGEST_MODE_H_
#define DOMAIN_TYPES_INGEST_MODE_H_

enum class IngestMode {
  kStandard = 0,
  kSingleTxtReplaceMonth = 1,
};

namespace tracer::core::domain::types {

#include "domain/detail/ingest_mode_contract.inc"

}  // namespace tracer::core::domain::types

#endif  // DOMAIN_TYPES_INGEST_MODE_H_
