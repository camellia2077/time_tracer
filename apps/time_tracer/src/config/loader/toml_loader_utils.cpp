// config/loader/toml_loader_utils.cpp
#include "toml_loader_utils.hpp"

#include <iostream>
#include <stdexcept>

#include "io/core/file_reader.hpp"

namespace TomlLoaderUtils {

auto read_toml(const fs::path& path) -> toml::table {
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

void parse_statistics_items(const toml::array* arr,
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
    item.label = get_required<std::string>(tbl, "label");
    item.show = tbl["show"].value_or(true);
    item.db_column = tbl["db_column"].value_or<std::string>("");

    if (const toml::array* sub_arr = tbl["sub_items"].as_array()) {
      parse_statistics_items(sub_arr, item.sub_items);
    }
    out_items.push_back(item);
  }
}

void fill_tex_style(const toml::table& tbl, FontConfig& fonts,
                    LayoutConfig& layout) {
  fonts.main_font = get_required<std::string>(tbl, "main_font");
  fonts.cjk_main_font = tbl["cjk_main_font"].value_or(fonts.main_font);

  fonts.base_font_size = get_required<int>(tbl, "base_font_size");
  fonts.report_title_font_size =
      get_required<int>(tbl, "report_title_font_size");
  fonts.category_title_font_size =
      get_required<int>(tbl, "category_title_font_size");

  layout.margin_in = tbl["margin_in"].value_or(1.0);
  layout.list_top_sep_pt = tbl["list_top_sep_pt"].value_or(0.0);
  layout.list_item_sep_ex = tbl["list_item_sep_ex"].value_or(0.0);
}

void fill_typ_style(const toml::table& tbl, FontConfig& fonts,
                    LayoutConfig& layout) {
  fonts.base_font = get_required<std::string>(tbl, "base_font");
  fonts.title_font = tbl["title_font"].value_or(fonts.base_font);
  fonts.category_title_font =
      tbl["category_title_font"].value_or(fonts.base_font);

  fonts.base_font_size = get_required<int>(tbl, "base_font_size");
  fonts.report_title_font_size =
      get_required<int>(tbl, "report_title_font_size");
  fonts.category_title_font_size =
      get_required<int>(tbl, "category_title_font_size");

  layout.line_spacing_em = tbl["line_spacing_em"].value_or(0.65);
  layout.margin_top_cm = tbl["margin_top_cm"].value_or(2.5);
  layout.margin_bottom_cm = tbl["margin_bottom_cm"].value_or(2.5);
  layout.margin_left_cm = tbl["margin_left_cm"].value_or(2.0);
  layout.margin_right_cm = tbl["margin_right_cm"].value_or(2.0);
}

void fill_daily_labels(const toml::table& tbl, DailyReportLabels& labels) {
  if (auto val = tbl["title_prefix"].value<std::string>()) {
    labels.report_title_prefix = *val;
  } else {
    labels.report_title_prefix =
        tbl["report_title"].value_or("Daily Report for");
  }

  labels.date_label = get_required<std::string>(tbl, "date_label");
  labels.total_time_label = get_required<std::string>(tbl, "total_time_label");

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

void fill_range_labels(const toml::table& tbl, RangeReportLabels& labels) {
  labels.title_template = get_required<std::string>(tbl, "title_template");
  labels.total_time_label = get_required<std::string>(tbl, "total_time_label");
  labels.actual_days_label =
      get_required<std::string>(tbl, "actual_days_label");
  labels.no_records_message =
      get_required<std::string>(tbl, "no_records_message");
  labels.invalid_range_message =
      tbl["invalid_range_message"].value_or("Invalid range.");
  labels.project_breakdown_label =
      tbl["project_breakdown_label"].value_or("Project Breakdown");
}

void fill_monthly_labels(const toml::table& tbl, MonthlyReportLabels& labels) {
  fill_range_labels(tbl, labels);
}

void fill_period_labels(const toml::table& tbl, PeriodReportLabels& labels) {
  fill_range_labels(tbl, labels);
}

void fill_keyword_colors(const toml::table& tbl,
                         std::map<std::string, std::string>& colors) {
  if (const toml::table* color_tbl = tbl["keyword_colors"].as_table()) {
    for (const auto& [key, val] : *color_tbl) {
      if (auto s = val.value<std::string>()) {
        // [修复] 显式转换为 std::string
        colors[std::string(key.str())] = *s;
      }
    }
  }
}
}  // namespace TomlLoaderUtils
