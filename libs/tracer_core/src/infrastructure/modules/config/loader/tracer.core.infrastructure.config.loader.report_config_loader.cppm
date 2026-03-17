module;

#include <filesystem>
#include <string>
#include <vector>

#include "infrastructure/config/models/report_config_models.hpp"

export module tracer.core.infrastructure.config.loader.report_config_loader;

export namespace tracer::core::infrastructure::config {

#include "infrastructure/config/detail/report_config_loader_decl.inc"

}  // namespace tracer::core::infrastructure::config

export namespace tracer::core::infrastructure::modconfig {

using tracer::core::infrastructure::config::ReportConfigLoader;

}  // namespace tracer::core::infrastructure::modconfig
