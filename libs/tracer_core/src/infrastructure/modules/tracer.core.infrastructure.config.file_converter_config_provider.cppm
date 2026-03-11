module;

#include <filesystem>
#include <optional>
#include <unordered_map>

#include "application/ports/i_converter_config_provider.hpp"

export module tracer.core.infrastructure.config.file_converter_config_provider;

export namespace tracer::core::infrastructure::config {

#include "infrastructure/config/detail/file_converter_config_provider_decl.inc"

}  // namespace tracer::core::infrastructure::config

export namespace tracer::core::infrastructure::modconfig {

using tracer::core::infrastructure::config::FileConverterConfigProvider;

}  // namespace tracer::core::infrastructure::modconfig
