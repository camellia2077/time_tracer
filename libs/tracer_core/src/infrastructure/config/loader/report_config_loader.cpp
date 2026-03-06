// infrastructure/config/loader/report_config_loader.cpp
#include "infrastructure/config/loader/report_config_loader.hpp"

#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

#include "infrastructure/config/loader/toml_loader_utils.hpp"

using namespace TomlLoaderUtils;

#include "infrastructure/config/loader/internal/report_config_loader_namespace.inc"


// ==========================================
// Daily Loaders
// ==========================================

auto ReportConfigLoader::LoadDailyTexConfig(const fs::path& path)
    -> DailyTexConfig {
  toml::table tbl = ReadToml(path);
  ValidateTexStyle(tbl, path);
  ValidateDailyLabels(tbl, path, "report_title");
  ValidateKeywordColors(tbl, path);
  ValidateDailyStatistics(tbl, path);

  DailyTexConfig config;
  FillTexStyle(tbl, config.fonts, config.layout);
  FillKeywordColors(tbl, config.keyword_colors);
  FillDailyLabels(tbl, config.labels);
  ParseStatisticsItems(tbl["statistics_items"].as_array(),
                       config.statistics_items);
  return config;
}

auto ReportConfigLoader::LoadDailyTypConfig(const fs::path& path)
    -> DailyTypConfig {
  toml::table tbl = ReadToml(path);
  ValidateTypStyle(tbl, path);
  ValidateDailyLabels(tbl, path, "title_prefix");
  ValidateKeywordColors(tbl, path);
  ValidateDailyStatistics(tbl, path);
  ValidateOptionalPositiveIntegerIfPresent(tbl, path, "statistic_font_size");
  ValidateOptionalPositiveIntegerIfPresent(tbl, path,
                                           "statistic_title_font_size");

  DailyTypConfig config;
  FillTypStyle(tbl, config.fonts, config.layout);
  FillKeywordColors(tbl, config.keyword_colors);
  FillDailyLabels(tbl, config.labels);
  ParseStatisticsItems(tbl["statistics_items"].as_array(),
                       config.statistics_items);
  config.statistic_font_size =
      tbl["statistic_font_size"].value_or(kDefaultStatisticFontSize);
  config.statistic_title_font_size =
      tbl["statistic_title_font_size"].value_or(kDefaultStatisticTitleFontSize);
  return config;
}

auto ReportConfigLoader::LoadDailyMdConfig(const fs::path& path)
    -> DailyMdConfig {
  toml::table tbl = ReadToml(path);
  ValidateDailyLabels(tbl, path, "title_prefix");
  ValidateDailyStatistics(tbl, path);

  DailyMdConfig config;
  FillDailyLabels(tbl, config.labels);
  ParseStatisticsItems(tbl["statistics_items"].as_array(),
                       config.statistics_items);
  return config;
}

// ==========================================
// Monthly Loaders
// ==========================================

auto ReportConfigLoader::LoadMonthlyTexConfig(const fs::path& path)
    -> MonthlyTexConfig {
  toml::table tbl = ReadToml(path);
  ValidateTexStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  MonthlyTexConfig config;
  FillTexStyle(tbl, config.fonts, config.layout);
  FillMonthlyLabels(tbl, config.labels);
  return config;
}

auto ReportConfigLoader::LoadMonthlyTypConfig(const fs::path& path)
    -> MonthlyTypConfig {
  toml::table tbl = ReadToml(path);
  ValidateTypStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  MonthlyTypConfig config;
  FillTypStyle(tbl, config.fonts, config.layout);
  FillMonthlyLabels(tbl, config.labels);
  return config;
}

auto ReportConfigLoader::LoadMonthlyMdConfig(const fs::path& path)
    -> MonthlyMdConfig {
  toml::table tbl = ReadToml(path);
  ValidateRequiredPeriodLabels(tbl, path);

  MonthlyMdConfig config;
  FillMonthlyLabels(tbl, config.labels);
  return config;
}

// ==========================================
// Period Loaders
// ==========================================

auto ReportConfigLoader::LoadPeriodTexConfig(const fs::path& path)
    -> PeriodTexConfig {
  toml::table tbl = ReadToml(path);
  ValidateTexStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  PeriodTexConfig config;
  FillTexStyle(tbl, config.fonts, config.layout);
  FillPeriodLabels(tbl, config.labels);
  return config;
}

auto ReportConfigLoader::LoadPeriodTypConfig(const fs::path& path)
    -> PeriodTypConfig {
  toml::table tbl = ReadToml(path);
  ValidateTypStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  PeriodTypConfig config;
  FillTypStyle(tbl, config.fonts, config.layout);
  FillPeriodLabels(tbl, config.labels);
  return config;
}

auto ReportConfigLoader::LoadPeriodMdConfig(const fs::path& path)
    -> PeriodMdConfig {
  toml::table tbl = ReadToml(path);
  ValidateRequiredPeriodLabels(tbl, path);

  PeriodMdConfig config;
  FillPeriodLabels(tbl, config.labels);
  return config;
}

// ==========================================
// Weekly Loaders
// ==========================================

auto ReportConfigLoader::LoadWeeklyTexConfig(const fs::path& path)
    -> WeeklyTexConfig {
  toml::table tbl = ReadToml(path);
  ValidateTexStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  WeeklyTexConfig config;
  FillTexStyle(tbl, config.fonts, config.layout);
  FillWeeklyLabels(tbl, config.labels);
  return config;
}

auto ReportConfigLoader::LoadWeeklyTypConfig(const fs::path& path)
    -> WeeklyTypConfig {
  toml::table tbl = ReadToml(path);
  ValidateTypStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  WeeklyTypConfig config;
  FillTypStyle(tbl, config.fonts, config.layout);
  FillWeeklyLabels(tbl, config.labels);
  return config;
}

auto ReportConfigLoader::LoadWeeklyMdConfig(const fs::path& path)
    -> WeeklyMdConfig {
  toml::table tbl = ReadToml(path);
  ValidateRequiredPeriodLabels(tbl, path);

  WeeklyMdConfig config;
  FillWeeklyLabels(tbl, config.labels);
  return config;
}

// ==========================================
// Yearly Loaders
// ==========================================

auto ReportConfigLoader::LoadYearlyTexConfig(const fs::path& path)
    -> YearlyTexConfig {
  toml::table tbl = ReadToml(path);
  ValidateTexStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  YearlyTexConfig config;
  FillTexStyle(tbl, config.fonts, config.layout);
  FillYearlyLabels(tbl, config.labels);
  return config;
}

auto ReportConfigLoader::LoadYearlyTypConfig(const fs::path& path)
    -> YearlyTypConfig {
  toml::table tbl = ReadToml(path);
  ValidateTypStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  YearlyTypConfig config;
  FillTypStyle(tbl, config.fonts, config.layout);
  FillYearlyLabels(tbl, config.labels);
  return config;
}

auto ReportConfigLoader::LoadYearlyMdConfig(const fs::path& path)
    -> YearlyMdConfig {
  toml::table tbl = ReadToml(path);
  ValidateRequiredPeriodLabels(tbl, path);

  YearlyMdConfig config;
  FillYearlyLabels(tbl, config.labels);
  return config;
}
