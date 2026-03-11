// infrastructure/config/static_converter_config_provider.hpp
#ifndef INFRASTRUCTURE_CONFIG_STATIC_CONVERTER_CONFIG_PROVIDER_H_
#define INFRASTRUCTURE_CONFIG_STATIC_CONVERTER_CONFIG_PROVIDER_H_

#include "application/ports/i_converter_config_provider.hpp"

namespace tracer::core::infrastructure::config {

#include "infrastructure/config/detail/static_converter_config_provider_decl.inc"

}  // namespace tracer::core::infrastructure::config

namespace infrastructure::config {

using tracer::core::infrastructure::config::StaticConverterConfigProvider;

}  // namespace infrastructure::config

#endif  // INFRASTRUCTURE_CONFIG_STATIC_CONVERTER_CONFIG_PROVIDER_H_
