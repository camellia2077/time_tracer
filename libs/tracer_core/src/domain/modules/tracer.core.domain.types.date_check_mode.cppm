module;

#include "domain/types/date_check_mode.hpp"

export module tracer.core.domain.types.date_check_mode;

export namespace tracer::core::domain::types {

#include "domain/detail/date_check_mode_contract.inc"

}  // namespace tracer::core::domain::types

export namespace tracer::core::domain::modtypes {

using tracer::core::domain::types::DateCheckMode;

}  // namespace tracer::core::domain::modtypes
