// infrastructure/config/file_converter_config_provider.hpp
#ifndef INFRASTRUCTURE_CONFIG_FILE_CONVERTER_CONFIG_PROVIDER_H_
#define INFRASTRUCTURE_CONFIG_FILE_CONVERTER_CONFIG_PROVIDER_H_

#include <filesystem>
#include <optional>
#include <unordered_map>

#include "application/ports/i_converter_config_provider.hpp"

namespace tracer::core::infrastructure::config {

#include "infrastructure/config/detail/file_converter_config_provider_decl.inc"

}  // namespace tracer::core::infrastructure::config

namespace infrastructure::config {

using tracer::core::infrastructure::config::FileConverterConfigProvider;

}  // namespace infrastructure::config

#endif  // INFRASTRUCTURE_CONFIG_FILE_CONVERTER_CONFIG_PROVIDER_H_
