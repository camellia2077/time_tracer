module;

#include "infrastructure/config/loader/toml_loader_utils.hpp"

export module tracer.core.infrastructure.config.loader.toml_loader_utils;

export namespace tracer::core::infrastructure::modconfig::loader {

using ::TomlLoaderUtils::FillDailyLabels;
using ::TomlLoaderUtils::FillKeywordColors;
using ::TomlLoaderUtils::FillMonthlyLabels;
using ::TomlLoaderUtils::FillPeriodLabels;
using ::TomlLoaderUtils::FillRangeLabels;
using ::TomlLoaderUtils::FillTexStyle;
using ::TomlLoaderUtils::FillTypStyle;
using ::TomlLoaderUtils::FillWeeklyLabels;
using ::TomlLoaderUtils::FillYearlyLabels;
using ::TomlLoaderUtils::ParseStatisticsItems;
using ::TomlLoaderUtils::ReadToml;

}  // namespace tracer::core::infrastructure::modconfig::loader
