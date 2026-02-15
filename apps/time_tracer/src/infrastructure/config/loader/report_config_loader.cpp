// infrastructure/config/loader/report_config_loader.cpp
#include "infrastructure/config/loader/report_config_loader.hpp"

#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

#include "infrastructure/config/loader/toml_loader_utils.hpp"

using namespace TomlLoaderUtils;

namespace {
constexpr int kDefaultStatisticFontSize = 10;
constexpr int kDefaultStatisticTitleFontSize = 12;

[[noreturn]] auto ThrowInvalidConfig(const fs::path& path,
                                     std::string_view detail) -> void {
  throw std::runtime_error("Invalid report config [" + path.string() +
                           "]: " + std::string(detail));
}

auto RequireNode(const toml::table& tbl, const fs::path& path,
                 std::string_view key) -> toml::node_view<const toml::node> {
  const toml::node_view<const toml::node> node = tbl[key];
  if (!node) {
    ThrowInvalidConfig(path, "missing key '" + std::string(key) + "'.");
  }
  return node;
}

auto RequireString(const toml::table& tbl, const fs::path& path,
                   std::string_view key) -> void {
  const toml::node_view<const toml::node> node = RequireNode(tbl, path, key);
  if (!node.is_string()) {
    ThrowInvalidConfig(path,
                       "key '" + std::string(key) + "' must be a string.");
  }
  const auto value = node.value<std::string>();
  if (!value || value->empty()) {
    ThrowInvalidConfig(path,
                       "key '" + std::string(key) + "' must not be empty.");
  }
}

auto ValidateOptionalStringIfPresent(const toml::table& tbl,
                                     const fs::path& path, std::string_view key)
    -> void {
  const toml::node_view<const toml::node> node = tbl[key];
  if (!node) {
    return;
  }
  if (!node.is_string()) {
    ThrowInvalidConfig(
        path, "key '" + std::string(key) + "' must be a string when present.");
  }
  const auto value = node.value<std::string>();
  if (!value || value->empty()) {
    ThrowInvalidConfig(
        path, "key '" + std::string(key) + "' must not be empty when present.");
  }
}

auto RequirePositiveInteger(const toml::table& tbl, const fs::path& path,
                            std::string_view key) -> void {
  const toml::node_view<const toml::node> node = RequireNode(tbl, path, key);
  if (!node.is_integer()) {
    ThrowInvalidConfig(path,
                       "key '" + std::string(key) + "' must be an integer.");
  }
  const auto value = node.value<int>();
  if (!value || *value <= 0) {
    ThrowInvalidConfig(path, "key '" + std::string(key) + "' must be > 0.");
  }
}

auto ValidateOptionalPositiveIntegerIfPresent(const toml::table& tbl,
                                              const fs::path& path,
                                              std::string_view key) -> void {
  const toml::node_view<const toml::node> node = tbl[key];
  if (!node) {
    return;
  }
  if (!node.is_integer()) {
    ThrowInvalidConfig(path, "key '" + std::string(key) +
                                 "' must be an integer when present.");
  }
  const auto value = node.value<int>();
  if (!value || *value <= 0) {
    ThrowInvalidConfig(
        path, "key '" + std::string(key) + "' must be > 0 when present.");
  }
}

auto ValidateOptionalNumberIfPresent(const toml::table& tbl,
                                     const fs::path& path, std::string_view key)
    -> void {
  const toml::node_view<const toml::node> node = tbl[key];
  if (!node) {
    return;
  }
  if (!node.is_number()) {
    ThrowInvalidConfig(
        path, "key '" + std::string(key) + "' must be numeric when present.");
  }
}

auto IsValidHexColor(std::string_view color) -> bool {
  if (color.size() != 7 || color[0] != '#') {
    return false;
  }
  for (size_t index = 1; index < color.size(); ++index) {
    if (std::isxdigit(static_cast<unsigned char>(color[index])) == 0) {
      return false;
    }
  }
  return true;
}

auto ValidateStatisticsItemsArray(const toml::array& items,
                                  const fs::path& path,
                                  std::string_view context) -> void {
  if (items.empty()) {
    ThrowInvalidConfig(path, std::string(context) + " must not be empty.");
  }
  for (size_t index = 0; index < items.size(); ++index) {
    const toml::node& item_node = items[index];
    const std::string item_context =
        std::string(context) + "[" + std::to_string(index) + "]";
    if (!item_node.is_table()) {
      ThrowInvalidConfig(path, item_context + " must be a table.");
    }
    const toml::table& item_tbl = *item_node.as_table();

    const toml::node_view<const toml::node> label_node = item_tbl["label"];
    if (!label_node || !label_node.is_string()) {
      ThrowInvalidConfig(path, item_context + ".label must be a string.");
    }
    const auto label_value = label_node.value<std::string>();
    if (!label_value || label_value->empty()) {
      ThrowInvalidConfig(path, item_context + ".label must not be empty.");
    }

    const toml::node_view<const toml::node> show_node = item_tbl["show"];
    if (show_node && !show_node.is_boolean()) {
      ThrowInvalidConfig(path, item_context + ".show must be a boolean.");
    }

    const toml::node_view<const toml::node> column_node = item_tbl["db_column"];
    if (column_node && !column_node.is_string()) {
      ThrowInvalidConfig(path, item_context + ".db_column must be a string.");
    }

    const toml::node_view<const toml::node> sub_items_node =
        item_tbl["sub_items"];
    if (!sub_items_node) {
      continue;
    }
    if (!sub_items_node.is_array()) {
      ThrowInvalidConfig(path, item_context + ".sub_items must be an array.");
    }
    ValidateStatisticsItemsArray(*sub_items_node.as_array(), path,
                                 item_context + ".sub_items");
  }
}

auto ValidateRequiredPeriodLabels(const toml::table& tbl, const fs::path& path)
    -> void {
  RequireString(tbl, path, "title_template");
  RequireString(tbl, path, "actual_days_label");
  RequireString(tbl, path, "status_days_label");
  RequireString(tbl, path, "sleep_days_label");
  RequireString(tbl, path, "exercise_days_label");
  RequireString(tbl, path, "cardio_days_label");
  RequireString(tbl, path, "anaerobic_days_label");
  RequireString(tbl, path, "total_time_label");
  RequireString(tbl, path, "no_records_message");
  RequireString(tbl, path, "invalid_range_message");
  RequireString(tbl, path, "project_breakdown_label");
}

auto ValidateDailyLabels(const toml::table& tbl, const fs::path& path,
                         std::string_view title_key) -> void {
  RequireString(tbl, path, title_key);
  RequireString(tbl, path, "date_label");
  RequireString(tbl, path, "total_time_label");
  RequireString(tbl, path, "status_label");
  RequireString(tbl, path, "sleep_label");
  RequireString(tbl, path, "exercise_label");
  RequireString(tbl, path, "getup_time_label");
  RequireString(tbl, path, "remark_label");
  RequireString(tbl, path, "statistics_label");
  RequireString(tbl, path, "all_activities_label");
  RequireString(tbl, path, "activity_remark_label");
  RequireString(tbl, path, "activity_connector");
  RequireString(tbl, path, "no_records_message");
}

auto ValidateDailyStatistics(const toml::table& tbl, const fs::path& path)
    -> void {
  const toml::node_view<const toml::node> stats_node =
      RequireNode(tbl, path, "statistics_items");
  if (!stats_node.is_array()) {
    ThrowInvalidConfig(path, "key 'statistics_items' must be an array.");
  }
  ValidateStatisticsItemsArray(*stats_node.as_array(), path,
                               "statistics_items");
}

auto ValidateKeywordColors(const toml::table& tbl, const fs::path& path)
    -> void {
  const toml::node_view<const toml::node> color_node =
      RequireNode(tbl, path, "keyword_colors");
  if (!color_node.is_table()) {
    ThrowInvalidConfig(path, "key 'keyword_colors' must be a table.");
  }

  const toml::table& colors = *color_node.as_table();
  if (colors.empty()) {
    ThrowInvalidConfig(path, "key 'keyword_colors' must not be empty.");
  }

  for (const auto& [color_key, color_value_node] : colors) {
    if (!color_value_node.is_string()) {
      ThrowInvalidConfig(path, "keyword_colors." +
                                   std::string(color_key.str()) +
                                   " must be a string.");
    }
    const auto color_value = color_value_node.value<std::string>();
    if (!color_value || !IsValidHexColor(*color_value)) {
      ThrowInvalidConfig(path, "keyword_colors." +
                                   std::string(color_key.str()) +
                                   " must be a hex color like #RRGGBB.");
    }
  }
}

auto ValidateTexStyle(const toml::table& tbl, const fs::path& path) -> void {
  RequireString(tbl, path, "main_font");
  ValidateOptionalStringIfPresent(tbl, path, "cjk_main_font");
  RequirePositiveInteger(tbl, path, "base_font_size");
  RequirePositiveInteger(tbl, path, "report_title_font_size");
  RequirePositiveInteger(tbl, path, "category_title_font_size");
  ValidateOptionalNumberIfPresent(tbl, path, "margin_in");
  ValidateOptionalNumberIfPresent(tbl, path, "list_top_sep_pt");
  ValidateOptionalNumberIfPresent(tbl, path, "list_item_sep_ex");
}

auto ValidateTypStyle(const toml::table& tbl, const fs::path& path) -> void {
  RequireString(tbl, path, "base_font");
  ValidateOptionalStringIfPresent(tbl, path, "title_font");
  ValidateOptionalStringIfPresent(tbl, path, "category_title_font");
  RequirePositiveInteger(tbl, path, "base_font_size");
  RequirePositiveInteger(tbl, path, "report_title_font_size");
  RequirePositiveInteger(tbl, path, "category_title_font_size");
  ValidateOptionalNumberIfPresent(tbl, path, "line_spacing_em");
  ValidateOptionalNumberIfPresent(tbl, path, "margin_top_cm");
  ValidateOptionalNumberIfPresent(tbl, path, "margin_bottom_cm");
  ValidateOptionalNumberIfPresent(tbl, path, "margin_left_cm");
  ValidateOptionalNumberIfPresent(tbl, path, "margin_right_cm");
}

}  // namespace

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
