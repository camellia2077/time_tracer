#include "infra/config/loader/insights_config_loader.hpp"

#include <toml++/toml.h>

#include <cctype>
#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

import tracer.core.infrastructure.config.loader.toml_loader_utils;

namespace fs = std::filesystem;
namespace modloader = tracer::core::infrastructure::config::loader;

#include "infra/config/loader/internal/insights_config_loader_namespace.inc"

namespace tracer::core::infrastructure::config {

// ==========================================
// Daily Loaders
// ==========================================

auto InsightsConfigLoader::LoadDailyTexConfig(const fs::path& path)
    -> DailyTexConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateTexStyle(tbl, path);
  ValidateDailyLabels(tbl, path, "insights_title");
  ValidateKeywordColors(tbl, path);
  ValidateDailyStatistics(tbl, path);

  DailyTexConfig config;
  modloader::FillTexStyle(tbl, config.fonts, config.layout);
  modloader::FillKeywordColors(tbl, config.keyword_colors);
  modloader::FillDailyLabels(tbl, config.labels);
  modloader::ParseStatisticsItems(tbl["statistics_items"].as_array(),
                                  config.statistics_items);
  return config;
}

auto InsightsConfigLoader::LoadDailyTypConfig(const fs::path& path)
    -> DailyTypConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateTypStyle(tbl, path);
  ValidateDailyLabels(tbl, path, "title_prefix");
  ValidateKeywordColors(tbl, path);
  ValidateDailyStatistics(tbl, path);
  ValidateOptionalPositiveIntegerIfPresent(tbl, path, "statistic_font_size");
  ValidateOptionalPositiveIntegerIfPresent(tbl, path,
                                           "statistic_title_font_size");

  DailyTypConfig config;
  modloader::FillTypStyle(tbl, config.fonts, config.layout);
  modloader::FillKeywordColors(tbl, config.keyword_colors);
  modloader::FillDailyLabels(tbl, config.labels);
  modloader::ParseStatisticsItems(tbl["statistics_items"].as_array(),
                                  config.statistics_items);
  config.statistic_font_size =
      tbl["statistic_font_size"].value_or(kDefaultStatisticFontSize);
  config.statistic_title_font_size =
      tbl["statistic_title_font_size"].value_or(kDefaultStatisticTitleFontSize);
  return config;
}

auto InsightsConfigLoader::LoadDailyMdConfig(const fs::path& path)
    -> DailyMdConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateDailyLabels(tbl, path, "title_prefix", false);

  DailyMdConfig config;
  modloader::FillDailyLabels(tbl, config.labels);
  config.end_only_time_format =
      tbl["end_only_time_format"].value_or("As of {end_time}");
  return config;
}

auto InsightsConfigLoader::LoadDailyStatusConfig(const fs::path& path)
    -> DailyStatusConfig {
  toml::table tbl = modloader::ReadToml(path);
  const auto kSchemaVersion = tbl["schema_version"].value<int>();
  if (!kSchemaVersion || *kSchemaVersion != 1) {
    ThrowInvalidConfig(path, "key 'schema_version' must be integer 1.");
  }

  const toml::node_view<const toml::node> kStatusesNode =
      RequireNode(tbl, path, "daily_statuses");
  if (!kStatusesNode.is_table()) {
    ThrowInvalidConfig(path, "key 'daily_statuses' must be a table.");
  }
  const toml::table& statuses = *kStatusesNode.as_table();
  const toml::node_view<const toml::node> kParentPresentNode =
      RequireNode(statuses, path, "parent_present");
  if (!kParentPresentNode.is_table()) {
    ThrowInvalidConfig(path,
                       "key 'daily_statuses.parent_present' must be a table.");
  }

  DailyStatusConfig config;
  config.schema_version = *kSchemaVersion;
  for (const auto& [id_node, status_node] : *kParentPresentNode.as_table()) {
    const std::string kId = std::string(id_node.str());
    if (!status_node.is_table()) {
      ThrowInvalidConfig(
          path, "daily_statuses.parent_present." + kId + " must be a table.");
    }
    const toml::table& status = *status_node.as_table();
    DailyStatusDefinition definition;
    definition.id = kId;
    definition.label = RequireNonEmptyString(status, path, "label");
    definition.parent = RequireNonEmptyString(status, path, "parent");
    definition.type = DailyStatusType::kParentPresent;
    config.statuses.push_back(std::move(definition));
  }
  return config;
}

// ==========================================
// Monthly Loaders
// ==========================================

auto InsightsConfigLoader::LoadMonthlyTexConfig(const fs::path& path)
    -> MonthlyTexConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateTexStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  MonthlyTexConfig config;
  modloader::FillTexStyle(tbl, config.fonts, config.layout);
  modloader::FillMonthlyLabels(tbl, config.labels);
  return config;
}

auto InsightsConfigLoader::LoadMonthlyTypConfig(const fs::path& path)
    -> MonthlyTypConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateTypStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  MonthlyTypConfig config;
  modloader::FillTypStyle(tbl, config.fonts, config.layout);
  modloader::FillMonthlyLabels(tbl, config.labels);
  return config;
}

auto InsightsConfigLoader::LoadMonthlyMdConfig(const fs::path& path)
    -> MonthlyMdConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateRequiredPeriodLabels(tbl, path);

  MonthlyMdConfig config;
  modloader::FillMonthlyLabels(tbl, config.labels);
  return config;
}

// ==========================================
// Period Loaders
// ==========================================

auto InsightsConfigLoader::LoadPeriodTexConfig(const fs::path& path)
    -> PeriodTexConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateTexStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  PeriodTexConfig config;
  modloader::FillTexStyle(tbl, config.fonts, config.layout);
  modloader::FillPeriodLabels(tbl, config.labels);
  return config;
}

auto InsightsConfigLoader::LoadPeriodTypConfig(const fs::path& path)
    -> PeriodTypConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateTypStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  PeriodTypConfig config;
  modloader::FillTypStyle(tbl, config.fonts, config.layout);
  modloader::FillPeriodLabels(tbl, config.labels);
  return config;
}

auto InsightsConfigLoader::LoadPeriodMdConfig(const fs::path& path)
    -> PeriodMdConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateRequiredPeriodLabels(tbl, path);

  PeriodMdConfig config;
  modloader::FillPeriodLabels(tbl, config.labels);
  return config;
}

// ==========================================
// Weekly Loaders
// ==========================================

auto InsightsConfigLoader::LoadWeeklyTexConfig(const fs::path& path)
    -> WeeklyTexConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateTexStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  WeeklyTexConfig config;
  modloader::FillTexStyle(tbl, config.fonts, config.layout);
  modloader::FillWeeklyLabels(tbl, config.labels);
  return config;
}

auto InsightsConfigLoader::LoadWeeklyTypConfig(const fs::path& path)
    -> WeeklyTypConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateTypStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  WeeklyTypConfig config;
  modloader::FillTypStyle(tbl, config.fonts, config.layout);
  modloader::FillWeeklyLabels(tbl, config.labels);
  return config;
}

auto InsightsConfigLoader::LoadWeeklyMdConfig(const fs::path& path)
    -> WeeklyMdConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateRequiredPeriodLabels(tbl, path);

  WeeklyMdConfig config;
  modloader::FillWeeklyLabels(tbl, config.labels);
  return config;
}

// ==========================================
// Yearly Loaders
// ==========================================

auto InsightsConfigLoader::LoadYearlyTexConfig(const fs::path& path)
    -> YearlyTexConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateTexStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  YearlyTexConfig config;
  modloader::FillTexStyle(tbl, config.fonts, config.layout);
  modloader::FillYearlyLabels(tbl, config.labels);
  return config;
}

auto InsightsConfigLoader::LoadYearlyTypConfig(const fs::path& path)
    -> YearlyTypConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateTypStyle(tbl, path);
  ValidateRequiredPeriodLabels(tbl, path);

  YearlyTypConfig config;
  modloader::FillTypStyle(tbl, config.fonts, config.layout);
  modloader::FillYearlyLabels(tbl, config.labels);
  return config;
}

auto InsightsConfigLoader::LoadYearlyMdConfig(const fs::path& path)
    -> YearlyMdConfig {
  toml::table tbl = modloader::ReadToml(path);
  ValidateRequiredPeriodLabels(tbl, path);

  YearlyMdConfig config;
  modloader::FillYearlyLabels(tbl, config.labels);
  return config;
}

}  // namespace tracer::core::infrastructure::config
