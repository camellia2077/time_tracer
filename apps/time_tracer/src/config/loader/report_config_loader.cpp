// config/loader/report_config_loader.cpp
#include "report_config_loader.hpp"

#include "toml_loader_utils.hpp"

using namespace TomlLoaderUtils;

// ==========================================
// Daily Loaders
// ==========================================

auto ReportConfigLoader::loadDailyTexConfig(const fs::path& path)
    -> DailyTexConfig {
  toml::table tbl = read_toml(path);
  DailyTexConfig config;

  fill_tex_style(tbl, config.fonts, config.layout);
  fill_keyword_colors(tbl, config.keyword_colors);
  fill_daily_labels(tbl, config.labels);

  if (const toml::array* arr = tbl["statistics_items"].as_array()) {
    parse_statistics_items(arr, config.statistics_items);
  }
  return config;
}

auto ReportConfigLoader::loadDailyTypConfig(const fs::path& path)
    -> DailyTypConfig {
  toml::table tbl = read_toml(path);
  DailyTypConfig config;

  fill_typ_style(tbl, config.fonts, config.layout);
  fill_keyword_colors(tbl, config.keyword_colors);
  fill_daily_labels(tbl, config.labels);

  if (const toml::array* arr = tbl["statistics_items"].as_array()) {
    parse_statistics_items(arr, config.statistics_items);
  }
  return config;
}

auto ReportConfigLoader::loadDailyMdConfig(const fs::path& path)
    -> DailyMdConfig {
  toml::table tbl = read_toml(path);
  DailyMdConfig config;

  fill_daily_labels(tbl, config.labels);

  if (const toml::array* arr = tbl["statistics_items"].as_array()) {
    parse_statistics_items(arr, config.statistics_items);
  }
  return config;
}

// ==========================================
// Monthly Loaders
// ==========================================

auto ReportConfigLoader::loadMonthlyTexConfig(const fs::path& path)
    -> MonthlyTexConfig {
  toml::table tbl = read_toml(path);
  MonthlyTexConfig config;

  fill_tex_style(tbl, config.fonts, config.layout);
  fill_monthly_labels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::loadMonthlyTypConfig(const fs::path& path)
    -> MonthlyTypConfig {
  toml::table tbl = read_toml(path);
  MonthlyTypConfig config;

  fill_typ_style(tbl, config.fonts, config.layout);
  fill_monthly_labels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::loadMonthlyMdConfig(const fs::path& path)
    -> MonthlyMdConfig {
  toml::table tbl = read_toml(path);
  MonthlyMdConfig config;

  fill_monthly_labels(tbl, config.labels);

  return config;
}

// ==========================================
// Period Loaders
// ==========================================

auto ReportConfigLoader::loadPeriodTexConfig(const fs::path& path)
    -> PeriodTexConfig {
  toml::table tbl = read_toml(path);
  PeriodTexConfig config;

  fill_tex_style(tbl, config.fonts, config.layout);
  fill_period_labels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::loadPeriodTypConfig(const fs::path& path)
    -> PeriodTypConfig {
  toml::table tbl = read_toml(path);
  PeriodTypConfig config;

  fill_typ_style(tbl, config.fonts, config.layout);
  fill_period_labels(tbl, config.labels);

  return config;
}

auto ReportConfigLoader::loadPeriodMdConfig(const fs::path& path)
    -> PeriodMdConfig {
  toml::table tbl = read_toml(path);
  PeriodMdConfig config;

  fill_period_labels(tbl, config.labels);

  return config;
}