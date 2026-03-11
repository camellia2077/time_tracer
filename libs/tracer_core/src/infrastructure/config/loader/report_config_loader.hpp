// infrastructure/config/loader/report_config_loader.hpp
#ifndef INFRASTRUCTURE_CONFIG_LOADER_REPORT_CONFIG_LOADER_H_
#define INFRASTRUCTURE_CONFIG_LOADER_REPORT_CONFIG_LOADER_H_

#include <filesystem>
#include <string>
#include <vector>

#include "infrastructure/config/models/report_config_models.hpp"

namespace tracer::core::infrastructure::config {

#include "infrastructure/config/detail/report_config_loader_decl.inc"

}  // namespace tracer::core::infrastructure::config

using ReportConfigLoader =
    tracer::core::infrastructure::config::ReportConfigLoader;

#endif  // INFRASTRUCTURE_CONFIG_LOADER_REPORT_CONFIG_LOADER_H_
