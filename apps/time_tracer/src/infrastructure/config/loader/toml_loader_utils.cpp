// infrastructure/config/loader/toml_loader_utils.cpp
#include "infrastructure/config/loader/toml_loader_utils.hpp"

#include <iostream>
#include <stdexcept>
#include <string_view>

#include "infrastructure/io/core/file_reader.hpp"

namespace {
constexpr double kTypLineSpacingEm = 0.65;
constexpr double kTypMarginTopBottomCm = 2.5;
constexpr double kTypMarginLeftRightCm = 2.0;
constexpr std::string_view kStyleSourceKey = "style_source";

auto ParseTomlFile(const fs::path& path) -> toml::table {
  try {
    return toml::parse_file(path.string());
  } catch (const toml::parse_error& e) {
    throw std::runtime_error("Config TOML Parse Error [" + path.string() +
                             "]: " + std::string(e.description()));
  } catch (const std::exception& e) {
    throw std::runtime_error("Config Load Error [" + path.string() +
                             "]: " + e.what());
  }
}

void MergeTomlTable(toml::table& target, const toml::table& source) {
  for (const auto& [key, val] : source) {
    if (target.contains(key)) {
      if (target[key].is_table() && val.is_table()) {
        MergeTomlTable(*target[key].as_table(), *val.as_table());
      } else {
        target.insert_or_assign(key, val);
      }
    } else {
      target.insert(key, val);
    }
  }
}

auto MergeStyleSource(const fs::path& path, toml::table config_tbl)
    -> toml::table {
  auto style_path = config_tbl[kStyleSourceKey].value<std::string>();
  if (!style_path) {
    return config_tbl;
  }

  fs::path style_file = path.parent_path() / *style_path;
  toml::table style_tbl = ParseTomlFile(style_file);
  MergeTomlTable(style_tbl, config_tbl);
  style_tbl.erase(std::string{kStyleSourceKey});
  return style_tbl;
}
}  // namespace

namespace TomlLoaderUtils {

auto ReadToml(const fs::path& path) -> toml::table {
  toml::table config_tbl = ParseTomlFile(path);
  return MergeStyleSource(path, std::move(config_tbl));
}

void ParseStatisticsItems(const toml::array* arr,
                          std::vector<ReportStatisticsItem>& out_items) {
  if (arr == nullptr) {
    return;
  }

  for (const auto& node : *arr) {
    if (!node.is_table()) {
      continue;
    }
    const auto& tbl = *node.as_table();

    ReportStatisticsItem item;
    item.label = GetRequired<std::string>(tbl, "label");
    item.show = tbl["show"].value_or(true);
    item.db_column = tbl["db_column"].value_or<std::string>("");

    if (const toml::array* sub_arr = tbl["sub_items"].as_array()) {
      ParseStatisticsItems(sub_arr, item.sub_items);
    }
    out_items.push_back(item);
  }
}

void FillTexStyle(const toml::table& tbl, FontConfig& fonts,
                  LayoutConfig& layout) {
  fonts.main_font = GetRequired<std::string>(tbl, "main_font");
  fonts.cjk_main_font = tbl["cjk_main_font"].value_or(fonts.main_font);

  fonts.base_font_size = GetRequired<int>(tbl, "base_font_size");
  fonts.report_title_font_size =
      GetRequired<int>(tbl, "report_title_font_size");
  fonts.category_title_font_size =
      GetRequired<int>(tbl, "category_title_font_size");

  layout.margin_in = tbl["margin_in"].value_or(1.0);
  layout.list_top_sep_pt = tbl["list_top_sep_pt"].value_or(0.0);
  layout.list_item_sep_ex = tbl["list_item_sep_ex"].value_or(0.0);
}

void FillTypStyle(const toml::table& tbl, FontConfig& fonts,
                  LayoutConfig& layout) {
  fonts.base_font = GetRequired<std::string>(tbl, "base_font");
  fonts.title_font = tbl["title_font"].value_or(fonts.base_font);
  fonts.category_title_font =
      tbl["category_title_font"].value_or(fonts.base_font);

  fonts.base_font_size = GetRequired<int>(tbl, "base_font_size");
  fonts.report_title_font_size =
      GetRequired<int>(tbl, "report_title_font_size");
  fonts.category_title_font_size =
      GetRequired<int>(tbl, "category_title_font_size");

  layout.line_spacing_em = tbl["line_spacing_em"].value_or(kTypLineSpacingEm);
  layout.margin_top_cm = tbl["margin_top_cm"].value_or(kTypMarginTopBottomCm);
  layout.margin_bottom_cm =
      tbl["margin_bottom_cm"].value_or(kTypMarginTopBottomCm);
  layout.margin_left_cm = tbl["margin_left_cm"].value_or(kTypMarginLeftRightCm);
  layout.margin_right_cm =
      tbl["margin_right_cm"].value_or(kTypMarginLeftRightCm);
}

void FillDailyLabels(const toml::table& tbl, DailyReportLabels& labels) {
  labels.report_title = tbl["report_title"].value_or("Daily Report for");

  if (auto val = tbl["title_prefix"].value<std::string>()) {
    labels.report_title_prefix = *val;
  } else {
    labels.report_title_prefix = labels.report_title;
  }

  labels.date_label = GetRequired<std::string>(tbl, "date_label");
  labels.total_time_label = GetRequired<std::string>(tbl, "total_time_label");

  labels.status_label = tbl["status_label"].value_or("Status");
  labels.sleep_label = tbl["sleep_label"].value_or("Sleep");
  labels.exercise_label = tbl["exercise_label"].value_or("Exercise");
  labels.getup_time_label = tbl["getup_time_label"].value_or("Getup Time");
  labels.remark_label = tbl["remark_label"].value_or("Remark");
  labels.no_records_message = tbl["no_records_message"].value_or("No records.");

  labels.statistics_label = tbl["statistics_label"].value_or("Statistics");
  labels.all_activities_label =
      tbl["all_activities_label"].value_or("All Activities");
  labels.activity_remark_label =
      tbl["activity_remark_label"].value_or("Remark");
  labels.project_breakdown_label =
      tbl["project_breakdown_label"].value_or("Project Breakdown");
  labels.activity_connector = tbl["activity_connector"].value_or("->");
}

void FillRangeLabels(const toml::table& tbl, RangeReportLabels& labels) {
  labels.report_title = tbl["report_title"].value_or("Monthly Report");
  labels.title_template = GetRequired<std::string>(tbl, "title_template");
  labels.total_time_label = GetRequired<std::string>(tbl, "total_time_label");
  labels.actual_days_label = GetRequired<std::string>(tbl, "actual_days_label");
  labels.status_days_label = tbl["status_days_label"].value_or("Status Days");
  labels.sleep_days_label = tbl["sleep_days_label"].value_or("Sleep Days");
  labels.exercise_days_label =
      tbl["exercise_days_label"].value_or("Exercise Days");
  labels.cardio_days_label = tbl["cardio_days_label"].value_or("Cardio Days");
  labels.anaerobic_days_label =
      tbl["anaerobic_days_label"].value_or("Anaerobic Days");
  labels.no_records_message =
      GetRequired<std::string>(tbl, "no_records_message");
  labels.invalid_format_message = tbl["invalid_format_message"].value_or("");
  labels.invalid_range_message =
      tbl["invalid_range_message"].value_or("Invalid range.");
  labels.project_breakdown_label =
      tbl["project_breakdown_label"].value_or("Project Breakdown");
}

void FillMonthlyLabels(const toml::table& tbl, MonthlyReportLabels& labels) {
  FillRangeLabels(tbl, labels);
}

void FillPeriodLabels(const toml::table& tbl, PeriodReportLabels& labels) {
  FillRangeLabels(tbl, labels);
}

void FillWeeklyLabels(const toml::table& tbl, WeeklyReportLabels& labels) {
  FillRangeLabels(tbl, labels);
}

void FillYearlyLabels(const toml::table& tbl, YearlyReportLabels& labels) {
  FillRangeLabels(tbl, labels);
}

void FillKeywordColors(const toml::table& tbl,
                       std::map<std::string, std::string>& colors) {
  if (const toml::table* color_tbl = tbl["keyword_colors"].as_table()) {
    for (const auto& [color_key, color_val] : *color_tbl) {
      if (auto hex_color = color_val.value<std::string>()) {
        // [修复] 显式转换为 std::string
        colors[std::string(color_key.str())] = *hex_color;
      }
    }
  }
}
}  // namespace TomlLoaderUtils
