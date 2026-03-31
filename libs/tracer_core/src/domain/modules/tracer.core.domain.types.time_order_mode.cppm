module;

#include "domain/types/time_order_mode.hpp"

export module tracer.core.domain.types.time_order_mode;

export namespace tracer::core::domain::types {

#include "domain/detail/time_order_mode_contract.inc"

}  // namespace tracer::core::domain::types

export namespace tracer::core::domain::modtypes {

using tracer::core::domain::types::TimeOrderMode;

}  // namespace tracer::core::domain::modtypes
