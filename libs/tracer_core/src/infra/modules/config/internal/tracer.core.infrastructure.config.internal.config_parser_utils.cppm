module;

#include <toml++/toml.h>

#include <filesystem>

#include "infra/config/models/app_config.hpp"

export module tracer.core.infrastructure.config.internal.config_parser_utils;

export namespace tracer::core::infrastructure::config::internal {

#include "infra/config/internal/detail/config_parser_utils_decl.inc"

}  // namespace tracer::core::infrastructure::config::internal

export namespace tracer::core::infrastructure::modconfig::internal {

using tracer::core::infrastructure::config::internal::ParseCliDefaults;
using tracer::core::infrastructure::config::internal::ParseSystemSettings;
using tracer::core::infrastructure::config::internal::ResolveBundlePath;
using tracer::core::infrastructure::config::internal::TryParseBundlePaths;

}  // namespace tracer::core::infrastructure::modconfig::internal
