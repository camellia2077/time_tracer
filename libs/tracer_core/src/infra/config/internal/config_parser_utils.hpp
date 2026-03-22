// infra/config/internal/config_parser_utils.hpp
#ifndef INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_PARSER_UTILS_H_
#define INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_PARSER_UTILS_H_

#include <toml++/toml.h>

#include <filesystem>

#include "infra/config/models/app_config.hpp"

namespace tracer::core::infrastructure::config::internal {

#include "infra/config/internal/detail/config_parser_utils_decl.inc"

}  // namespace tracer::core::infrastructure::config::internal

namespace ConfigParserUtils {

using tracer::core::infrastructure::config::internal::ParseCliDefaults;
using tracer::core::infrastructure::config::internal::ParseSystemSettings;
using tracer::core::infrastructure::config::internal::ResolveBundlePath;
using tracer::core::infrastructure::config::internal::TryParseBundlePaths;

}  // namespace ConfigParserUtils

#endif  // INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_PARSER_UTILS_H_
