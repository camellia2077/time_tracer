module;

#include <filesystem>
#include <string>

#include "infra/config/models/app_config.hpp"

export module tracer.core.infrastructure.config.config_loader;

export namespace tracer::core::infrastructure::config {

#include "infra/config/detail/config_loader_decl.inc"

}  // namespace tracer::core::infrastructure::config

export namespace tracer::core::infrastructure::modconfig {

using tracer::core::infrastructure::config::ConfigLoader;
using ::AppConfig;

}  // namespace tracer::core::infrastructure::modconfig
