// infra/config/loader/insights_config_loader.hpp
#ifndef INFRASTRUCTURE_CONFIG_LOADER_INSIGHTS_CONFIG_LOADER_H_
#define INFRASTRUCTURE_CONFIG_LOADER_INSIGHTS_CONFIG_LOADER_H_

#include <filesystem>
#include <string>
#include <vector>

#include "infra/config/models/insights_config_models.hpp"

namespace tracer::core::infrastructure::config {

#include "infra/config/detail/insights_config_loader_decl.inc"

}  // namespace tracer::core::infrastructure::config

using InsightsConfigLoader =
    tracer::core::infrastructure::config::InsightsConfigLoader;

#endif  // INFRASTRUCTURE_CONFIG_LOADER_INSIGHTS_CONFIG_LOADER_H_
