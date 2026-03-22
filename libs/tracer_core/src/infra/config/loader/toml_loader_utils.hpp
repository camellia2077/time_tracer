// infra/config/loader/toml_loader_utils.hpp
#ifndef INFRASTRUCTURE_CONFIG_LOADER_TOML_LOADER_UTILS_H_
#define INFRASTRUCTURE_CONFIG_LOADER_TOML_LOADER_UTILS_H_

#include <toml++/toml.h>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "infra/config/models/report_config_models.hpp"

namespace fs = std::filesystem;

namespace tracer::core::infrastructure::config::loader {

#include "infra/config/detail/toml_loader_utils_decl.inc"

}  // namespace tracer::core::infrastructure::config::loader

namespace TomlLoaderUtils {

using tracer::core::infrastructure::config::loader::FillDailyLabels;
using tracer::core::infrastructure::config::loader::FillKeywordColors;
using tracer::core::infrastructure::config::loader::FillMonthlyLabels;
using tracer::core::infrastructure::config::loader::FillPeriodLabels;
using tracer::core::infrastructure::config::loader::FillRangeLabels;
using tracer::core::infrastructure::config::loader::FillTexStyle;
using tracer::core::infrastructure::config::loader::FillTypStyle;
using tracer::core::infrastructure::config::loader::FillWeeklyLabels;
using tracer::core::infrastructure::config::loader::FillYearlyLabels;
using tracer::core::infrastructure::config::loader::GetOptional;
using tracer::core::infrastructure::config::loader::GetRequired;
using tracer::core::infrastructure::config::loader::ParseStatisticsItems;
using tracer::core::infrastructure::config::loader::ReadToml;

}  // namespace TomlLoaderUtils

#endif  // INFRASTRUCTURE_CONFIG_LOADER_TOML_LOADER_UTILS_H_
