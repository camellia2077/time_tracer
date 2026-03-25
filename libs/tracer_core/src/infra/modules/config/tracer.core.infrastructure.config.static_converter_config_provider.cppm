module;

#include "application/ports/pipeline/i_converter_config_provider.hpp"

export module tracer.core.infrastructure.config
    .static_converter_config_provider;

export namespace tracer::core::infrastructure::config {

#include "infra/config/detail/static_converter_config_provider_decl.inc"

}  // namespace tracer::core::infrastructure::config

export namespace tracer::core::infrastructure::modconfig {

using ::ConverterConfig;
using tracer::core::infrastructure::config::StaticConverterConfigProvider;

}  // namespace tracer::core::infrastructure::modconfig
