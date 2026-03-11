module;

#include <toml++/toml.h>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "infrastructure/config/models/report_config_models.hpp"

namespace fs = std::filesystem;

export module tracer.core.infrastructure.config.loader.toml_loader_utils;

export namespace tracer::core::infrastructure::config::loader {

#include "infrastructure/config/detail/toml_loader_utils_decl.inc"

}  // namespace tracer::core::infrastructure::config::loader

export namespace tracer::core::infrastructure::modconfig::loader {

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

}  // namespace tracer::core::infrastructure::modconfig::loader
