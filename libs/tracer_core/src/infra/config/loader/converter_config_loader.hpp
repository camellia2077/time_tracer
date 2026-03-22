// infra/config/loader/converter_config_loader.hpp
#ifndef INFRASTRUCTURE_CONFIG_LOADER_CONVERTER_CONFIG_LOADER_H_
#define INFRASTRUCTURE_CONFIG_LOADER_CONVERTER_CONFIG_LOADER_H_

#include <toml++/toml.h>

#include <filesystem>
#include <initializer_list>
#include <string_view>

#include "domain/types/converter_config.hpp"

namespace tracer::core::infrastructure::config {

#include "infra/config/detail/converter_config_loader_decl.inc"

}  // namespace tracer::core::infrastructure::config

using ConverterConfigLoader =
    tracer::core::infrastructure::config::ConverterConfigLoader;

#endif  // INFRASTRUCTURE_CONFIG_LOADER_CONVERTER_CONFIG_LOADER_H_
