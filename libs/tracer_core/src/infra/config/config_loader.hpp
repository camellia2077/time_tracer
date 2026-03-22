// infra/config/config_loader.hpp
#ifndef INFRASTRUCTURE_CONFIG_CONFIG_LOADER_H_
#define INFRASTRUCTURE_CONFIG_CONFIG_LOADER_H_

#include <filesystem>
#include <string>

#include "infra/config/models/app_config.hpp"

namespace tracer::core::infrastructure::config {

#include "infra/config/detail/config_loader_decl.inc"

}  // namespace tracer::core::infrastructure::config

using ConfigLoader = tracer::core::infrastructure::config::ConfigLoader;

#endif  // INFRASTRUCTURE_CONFIG_CONFIG_LOADER_H_
