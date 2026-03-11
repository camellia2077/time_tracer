module;

#include "domain/types/converter_config.hpp"

export module tracer.core.domain.types.converter_config;

export namespace tracer::core::domain::types {

#include "domain/detail/converter_config_contract.inc"

}  // namespace tracer::core::domain::types

export namespace tracer::core::domain::modtypes {

using tracer::core::domain::types::ConverterConfig;
using tracer::core::domain::types::DurationMappingRule;

}  // namespace tracer::core::domain::modtypes
