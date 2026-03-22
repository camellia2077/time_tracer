module;

#include <toml++/toml.h>

#include <filesystem>
#include <initializer_list>
#include <string_view>

#include "domain/types/converter_config.hpp"

export module tracer.core.infrastructure.config.loader.converter_config_loader;

export namespace tracer::core::infrastructure::config {

#include "infra/config/detail/converter_config_loader_decl.inc"

}  // namespace tracer::core::infrastructure::config

export namespace tracer::core::infrastructure::modconfig {

using tracer::core::infrastructure::config::ConverterConfigLoader;

}  // namespace tracer::core::infrastructure::modconfig
