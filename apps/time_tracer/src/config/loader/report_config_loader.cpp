// config/loader/report_config_loader.cpp
#include "config/loader/report_config_loader.hpp"

#include "config/loader/toml_loader_utils.hpp"

using namespace TomlLoaderUtils;

// ==========================================
// Daily Loaders
// ==========================================

auto ReportConfigLoader::LoadDailyTexConfig(const fs::path& path)
    -> DailyTexConfig {
  toml::table tbl = ReadToml(path);
  DailyTexConfig config;

  FillTexStyle(tbl, config.fonts, config.layout);
  FillKeywordColors(tbl, config.keyword_colors);
  FillDailyLabels(tbl, config.labels);

  if (const toml::array* arr = tbl["statistics_items"].as_array()) {
    ParseStatisticsItems(arr, config.statistics_items);
  }
  return config;
}

auto ReportConfigLoader::LoadDailyTypConfig(const fs::path& path)
    -> DailyTypConfig {
  toml::table tbl = ReadToml(path);
  DailyTypConfig config;

  FillTypStyle(tbl, config.fonts, config.layout);
  FillKeywordColors(tbl, config.keyword_colors);
  FillDailyLabels(tbl, config.labels);

  if (const toml::array* arr = tbl["statistics_items"].as_array()) {
    ParseStatisticsItems(arr, config.statistics_items);
  }
  return config;
}

auto ReportConfigLoader::LoadDailyMdConfig(const fs::path& path)
    -> DailyMdConfig {
  toml::table tbl = ReadToml(path);
  DailyMdConfig config;

  FillDailyLabels(tbl, config.labels);

  if (const toml::array* arr = tbl["statistics_items"].as_array()) {
    ParseStatisticsItems(arr, config.statistics_items);
  }
  return config;
}

// ==========================================
// Monthly Loaders
// ==========================================

auto ReportConfigLoader::LoadMonthlyTexConfig(const fs::path& path)
    -> MonthlyTexConfig {
  toml::table tbl = ReadToml(path);
  MonthlyTexConfig config;

  FillTexStyle(tbl, config.fonts, config.layout);
  FillMonthlyLabels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::LoadMonthlyTypConfig(const fs::path& path)
    -> MonthlyTypConfig {
  toml::table tbl = ReadToml(path);
  MonthlyTypConfig config;

  FillTypStyle(tbl, config.fonts, config.layout);
  FillMonthlyLabels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::LoadMonthlyMdConfig(const fs::path& path)
    -> MonthlyMdConfig {
  toml::table tbl = ReadToml(path);
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
  PeriodTexConfig config;

  FillTexStyle(tbl, config.fonts, config.layout);
  FillPeriodLabels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::LoadPeriodTypConfig(const fs::path& path)
    -> PeriodTypConfig {
  toml::table tbl = ReadToml(path);
  PeriodTypConfig config;

  FillTypStyle(tbl, config.fonts, config.layout);
  FillPeriodLabels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::LoadPeriodMdConfig(const fs::path& path)
    -> PeriodMdConfig {
  toml::table tbl = ReadToml(path);
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
  WeeklyTexConfig config;

  FillTexStyle(tbl, config.fonts, config.layout);
  FillWeeklyLabels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::LoadWeeklyTypConfig(const fs::path& path)
    -> WeeklyTypConfig {
  toml::table tbl = ReadToml(path);
  WeeklyTypConfig config;

  FillTypStyle(tbl, config.fonts, config.layout);
  FillWeeklyLabels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::LoadWeeklyMdConfig(const fs::path& path)
    -> WeeklyMdConfig {
  toml::table tbl = ReadToml(path);
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
  YearlyTexConfig config;

  FillTexStyle(tbl, config.fonts, config.layout);
  FillYearlyLabels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::LoadYearlyTypConfig(const fs::path& path)
    -> YearlyTypConfig {
  toml::table tbl = ReadToml(path);
  YearlyTypConfig config;

  FillTypStyle(tbl, config.fonts, config.layout);
  FillYearlyLabels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::LoadYearlyMdConfig(const fs::path& path)
    -> YearlyMdConfig {
  toml::table tbl = ReadToml(path);
  YearlyMdConfig config;

  FillYearlyLabels(tbl, config.labels);

  return config;
}
