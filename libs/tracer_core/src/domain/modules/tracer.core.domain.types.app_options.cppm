module;

#include "domain/types/app_options.hpp"

export module tracer.core.domain.types.app_options;

export namespace tracer::core::domain::types {

#include "domain/detail/app_options_contract.inc"

}  // namespace tracer::core::domain::types

export namespace tracer::core::domain::modtypes {

using tracer::core::domain::types::AppOptions;

}  // namespace tracer::core::domain::modtypes
