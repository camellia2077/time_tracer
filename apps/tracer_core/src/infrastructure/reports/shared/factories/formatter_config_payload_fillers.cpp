// infrastructure/reports/shared/factories/formatter_config_payload_fillers.cpp
#include <utility>

#include "infrastructure/reports/shared/factories/formatter_config_payload.hpp"

void FormatterConfigPayload::FlattenStatisticsItems(
    const toml::array* array, int32_t parent_index,
    std::vector<StatisticItemNodeOwned>* out) {
  if ((array == nullptr) || (out == nullptr)) {
    return;
  }

  for (const auto& node : *array) {
    const auto* table = node.as_table();
    if (table == nullptr) {
      continue;
    }

    StatisticItemNodeOwned item{};
    item.label =
        formatter_config_payload_detail::ReadString(*table, "label", "");
    item.db_column =
        formatter_config_payload_detail::ReadString(*table, "db_column", "");
    item.show = static_cast<uint8_t>((*table)["show"].value_or(true) ? 1U : 0U);
    item.parent_index = parent_index;

    out->push_back(std::move(item));
    const auto kCurrentIndex = static_cast<int32_t>(out->size() - 1U);

    if (const auto* sub_items = (*table)["sub_items"].as_array();
        sub_items != nullptr) {
      FlattenStatisticsItems(sub_items, kCurrentIndex, out);
    }
  }
}

void FormatterConfigPayload::FlattenStatisticsItems(
    const std::vector<ReportStatisticsItem>& items, int32_t parent_index,
    std::vector<StatisticItemNodeOwned>* out) {
  if (out == nullptr) {
    return;
  }

  for (const auto& item_source : items) {
    StatisticItemNodeOwned item{};
    item.label = item_source.label;
    item.db_column = item_source.db_column;
    item.show = static_cast<uint8_t>(item_source.show ? 1U : 0U);
    item.parent_index = parent_index;

    out->push_back(std::move(item));
    const auto kCurrentIndex = static_cast<int32_t>(out->size() - 1U);

    FlattenStatisticsItems(item_source.sub_items, kCurrentIndex, out);
  }
}

void FormatterConfigPayload::FillKeywordColors(
    const toml::table& table, std::vector<KeywordColorOwned>* out) {
  if (out == nullptr) {
    return;
  }
  out->clear();

  if (const auto* keyword_colors = table["keyword_colors"].as_table();
      keyword_colors != nullptr) {
    for (const auto& [key, value] : *keyword_colors) {
      if (auto color = value.value<std::string>(); color.has_value()) {
        out->push_back(
            KeywordColorOwned{.key = std::string(key.str()), .color = *color});
      }
    }
  }
}

void FormatterConfigPayload::FillKeywordColors(
    const std::map<std::string, std::string>& colors,
    std::vector<KeywordColorOwned>* out) {
  if (out == nullptr) {
    return;
  }
  out->clear();
  out->reserve(colors.size());

  for (const auto& [key, color] : colors) {
    out->push_back(KeywordColorOwned{.key = key, .color = color});
  }
}

void FormatterConfigPayload::FillDayLabels(const toml::table& table,
                                           DayLabelsOwned* out) {
  if (out == nullptr) {
    return;
  }

  if (auto title_prefix = table["title_prefix"].value<std::string>();
      title_prefix.has_value()) {
    out->title_prefix = *title_prefix;
  } else {
    out->title_prefix = formatter_config_payload_detail::ReadString(
        table, "report_title", "Daily Report for");
  }

  out->report_title = formatter_config_payload_detail::ReadString(
      table, "report_title", out->title_prefix);
  out->date_label =
      formatter_config_payload_detail::ReadString(table, "date_label", "");
  out->total_time_label = formatter_config_payload_detail::ReadString(
      table, "total_time_label", "");
  out->status_label = formatter_config_payload_detail::ReadString(
      table, "status_label", "Status");
  out->sleep_label = formatter_config_payload_detail::ReadString(
      table, "sleep_label", "Sleep");
  out->getup_time_label = formatter_config_payload_detail::ReadString(
      table, "getup_time_label", "Getup Time");
  out->remark_label = formatter_config_payload_detail::ReadString(
      table, "remark_label", "Remark");
  out->exercise_label = formatter_config_payload_detail::ReadString(
      table, "exercise_label", "Exercise");
  out->no_records_message = formatter_config_payload_detail::ReadString(
      table, "no_records_message", "No records.");
  out->statistics_label = formatter_config_payload_detail::ReadString(
      table, "statistics_label", "Statistics");
  out->all_activities_label = formatter_config_payload_detail::ReadString(
      table, "all_activities_label", "All Activities");
  out->activity_remark_label = formatter_config_payload_detail::ReadString(
      table, "activity_remark_label", "Remark");
  out->activity_connector = formatter_config_payload_detail::ReadString(
      table, "activity_connector", "->");
  out->project_breakdown_label = formatter_config_payload_detail::ReadString(
      table, "project_breakdown_label", "Project Breakdown");
}

void FormatterConfigPayload::FillDayLabels(const DailyReportLabels& labels,
                                           DayLabelsOwned* out) {
  if (out == nullptr) {
    return;
  }

  const std::string kReportTitle =
      labels.report_title.empty() ? "Daily Report for" : labels.report_title;
  out->title_prefix = labels.report_title_prefix.empty()
                          ? kReportTitle
                          : labels.report_title_prefix;
  out->report_title = kReportTitle.empty() ? out->title_prefix : kReportTitle;
  out->date_label = labels.date_label.empty() ? "Date" : labels.date_label;
  out->total_time_label = labels.total_time_label.empty()
                              ? "Total Time Recorded"
                              : labels.total_time_label;
  out->status_label =
      labels.status_label.empty() ? "Status" : labels.status_label;
  out->sleep_label = labels.sleep_label.empty() ? "Sleep" : labels.sleep_label;
  out->getup_time_label =
      labels.getup_time_label.empty() ? "Getup Time" : labels.getup_time_label;
  out->remark_label =
      labels.remark_label.empty() ? "Remark" : labels.remark_label;
  out->exercise_label =
      labels.exercise_label.empty() ? "Exercise" : labels.exercise_label;
  out->no_records_message = labels.no_records_message.empty()
                                ? "No records."
                                : labels.no_records_message;
  out->statistics_label =
      labels.statistics_label.empty() ? "Statistics" : labels.statistics_label;
  out->all_activities_label = labels.all_activities_label.empty()
                                  ? "All Activities"
                                  : labels.all_activities_label;
  out->activity_remark_label = labels.activity_remark_label.empty()
                                   ? "Remark"
                                   : labels.activity_remark_label;
  out->activity_connector =
      labels.activity_connector.empty() ? "->" : labels.activity_connector;
  out->project_breakdown_label = labels.project_breakdown_label.empty()
                                     ? "Project Breakdown"
                                     : labels.project_breakdown_label;
}

void FormatterConfigPayload::FillMonthLabels(const toml::table& table,
                                             MonthLabelsOwned* out) {
  if (out == nullptr) {
    return;
  }

  out->report_title =
      formatter_config_payload_detail::ReadString(table, "report_title", "");
  out->title_template = formatter_config_payload_detail::ReadString(
      table, "title_template", out->report_title);
  out->actual_days_label = formatter_config_payload_detail::ReadString(
      table, "actual_days_label", "");
  out->status_days_label = formatter_config_payload_detail::ReadString(
      table, "status_days_label", "Status Days");
  out->sleep_days_label = formatter_config_payload_detail::ReadString(
      table, "sleep_days_label", "Sleep Days");
  out->exercise_days_label = formatter_config_payload_detail::ReadString(
      table, "exercise_days_label", "Exercise Days");
  out->cardio_days_label = formatter_config_payload_detail::ReadString(
      table, "cardio_days_label", "Cardio Days");
  out->anaerobic_days_label = formatter_config_payload_detail::ReadString(
      table, "anaerobic_days_label", "Anaerobic Days");
  out->total_time_label = formatter_config_payload_detail::ReadString(
      table, "total_time_label", "");
  out->no_records_message = formatter_config_payload_detail::ReadString(
      table, "no_records_message", "");
  out->invalid_format_message = formatter_config_payload_detail::ReadString(
      table, "invalid_format_message", "");
  out->project_breakdown_label = formatter_config_payload_detail::ReadString(
      table, "project_breakdown_label", "Project Breakdown");
}

void FormatterConfigPayload::FillMonthLabels(const MonthlyReportLabels& labels,
                                             MonthLabelsOwned* out) {
  if (out == nullptr) {
    return;
  }

  out->report_title =
      labels.report_title.empty() ? "Monthly Report" : labels.report_title;
  out->title_template =
      labels.title_template.empty() ? out->report_title : labels.title_template;
  out->actual_days_label = labels.actual_days_label;
  out->status_days_label = labels.status_days_label.empty()
                               ? "Status Days"
                               : labels.status_days_label;
  out->sleep_days_label =
      labels.sleep_days_label.empty() ? "Sleep Days" : labels.sleep_days_label;
  out->exercise_days_label = labels.exercise_days_label.empty()
                                 ? "Exercise Days"
                                 : labels.exercise_days_label;
  out->cardio_days_label = labels.cardio_days_label.empty()
                               ? "Cardio Days"
                               : labels.cardio_days_label;
  out->anaerobic_days_label = labels.anaerobic_days_label.empty()
                                  ? "Anaerobic Days"
                                  : labels.anaerobic_days_label;
  out->total_time_label = labels.total_time_label;
  out->no_records_message = labels.no_records_message;
  out->invalid_format_message = labels.invalid_format_message;
  out->project_breakdown_label = labels.project_breakdown_label.empty()
                                     ? "Project Breakdown"
                                     : labels.project_breakdown_label;
}

void FormatterConfigPayload::FillRangeLabels(const toml::table& table,
                                             RangeLabelsOwned* out) {
  if (out == nullptr) {
    return;
  }

  out->title_template =
      formatter_config_payload_detail::ReadString(table, "title_template", "");
  out->actual_days_label = formatter_config_payload_detail::ReadString(
      table, "actual_days_label", "");
  out->status_days_label = formatter_config_payload_detail::ReadString(
      table, "status_days_label", "Status Days");
  out->sleep_days_label = formatter_config_payload_detail::ReadString(
      table, "sleep_days_label", "Sleep Days");
  out->exercise_days_label = formatter_config_payload_detail::ReadString(
      table, "exercise_days_label", "Exercise Days");
  out->cardio_days_label = formatter_config_payload_detail::ReadString(
      table, "cardio_days_label", "Cardio Days");
  out->anaerobic_days_label = formatter_config_payload_detail::ReadString(
      table, "anaerobic_days_label", "Anaerobic Days");
  out->total_time_label = formatter_config_payload_detail::ReadString(
      table, "total_time_label", "");
  out->no_records_message = formatter_config_payload_detail::ReadString(
      table, "no_records_message", "");
  out->invalid_range_message = formatter_config_payload_detail::ReadString(
      table, "invalid_range_message", "");
  out->project_breakdown_label = formatter_config_payload_detail::ReadString(
      table, "project_breakdown_label", "Project Breakdown");
}

void FormatterConfigPayload::FillRangeLabels(const RangeReportLabels& labels,
                                             RangeLabelsOwned* out) {
  if (out == nullptr) {
    return;
  }

  out->title_template = labels.title_template;
  out->actual_days_label = labels.actual_days_label;
  out->status_days_label = labels.status_days_label.empty()
                               ? "Status Days"
                               : labels.status_days_label;
  out->sleep_days_label =
      labels.sleep_days_label.empty() ? "Sleep Days" : labels.sleep_days_label;
  out->exercise_days_label = labels.exercise_days_label.empty()
                                 ? "Exercise Days"
                                 : labels.exercise_days_label;
  out->cardio_days_label = labels.cardio_days_label.empty()
                               ? "Cardio Days"
                               : labels.cardio_days_label;
  out->anaerobic_days_label = labels.anaerobic_days_label.empty()
                                  ? "Anaerobic Days"
                                  : labels.anaerobic_days_label;
  out->total_time_label = labels.total_time_label;
  out->no_records_message = labels.no_records_message;
  out->invalid_range_message = labels.invalid_range_message;
  out->project_breakdown_label = labels.project_breakdown_label.empty()
                                     ? "Project Breakdown"
                                     : labels.project_breakdown_label;
}

void FormatterConfigPayload::FillTexStyle(const toml::table& table,
                                          TexStyleOwned* out) {
  if (out == nullptr) {
    return;
  }

  out->main_font =
      formatter_config_payload_detail::ReadString(table, "main_font", "");
  out->cjk_main_font = formatter_config_payload_detail::ReadString(
      table, "cjk_main_font", out->main_font);
  out->base_font_size = formatter_config_payload_detail::ReadInt32(
      table, "base_font_size", out->base_font_size);
  out->report_title_font_size = formatter_config_payload_detail::ReadInt32(
      table, "report_title_font_size", out->report_title_font_size);
  out->category_title_font_size = formatter_config_payload_detail::ReadInt32(
      table, "category_title_font_size", out->category_title_font_size);
  out->margin_in = formatter_config_payload_detail::ReadDouble(
      table, "margin_in", out->margin_in);
  out->list_top_sep_pt = formatter_config_payload_detail::ReadDouble(
      table, "list_top_sep_pt", out->list_top_sep_pt);
  out->list_item_sep_ex = formatter_config_payload_detail::ReadDouble(
      table, "list_item_sep_ex", out->list_item_sep_ex);
}

void FormatterConfigPayload::FillTexStyle(const FontConfig& fonts,
                                          const LayoutConfig& layout,
                                          TexStyleOwned* out) {
  if (out == nullptr) {
    return;
  }

  out->main_font = fonts.main_font;
  out->cjk_main_font =
      fonts.cjk_main_font.empty() ? fonts.main_font : fonts.cjk_main_font;
  out->base_font_size = fonts.base_font_size;
  out->report_title_font_size = fonts.report_title_font_size;
  out->category_title_font_size = fonts.category_title_font_size;
  out->margin_in = layout.margin_in;
  out->list_top_sep_pt = layout.list_top_sep_pt;
  out->list_item_sep_ex = layout.list_item_sep_ex;
}

void FormatterConfigPayload::FillTypstStyle(const toml::table& table,
                                            TypstStyleOwned* out) {
  if (out == nullptr) {
    return;
  }

  out->base_font =
      formatter_config_payload_detail::ReadString(table, "base_font", "");
  out->title_font = formatter_config_payload_detail::ReadString(
      table, "title_font", out->base_font);
  out->category_title_font = formatter_config_payload_detail::ReadString(
      table, "category_title_font", out->base_font);
  out->base_font_size = formatter_config_payload_detail::ReadInt32(
      table, "base_font_size", out->base_font_size);
  out->report_title_font_size = formatter_config_payload_detail::ReadInt32(
      table, "report_title_font_size", out->report_title_font_size);
  out->category_title_font_size = formatter_config_payload_detail::ReadInt32(
      table, "category_title_font_size", out->category_title_font_size);
  out->line_spacing_em = formatter_config_payload_detail::ReadDouble(
      table, "line_spacing_em", out->line_spacing_em);
  out->margin_top_cm = formatter_config_payload_detail::ReadDouble(
      table, "margin_top_cm", out->margin_top_cm);
  out->margin_bottom_cm = formatter_config_payload_detail::ReadDouble(
      table, "margin_bottom_cm", out->margin_bottom_cm);
  out->margin_left_cm = formatter_config_payload_detail::ReadDouble(
      table, "margin_left_cm", out->margin_left_cm);
  out->margin_right_cm = formatter_config_payload_detail::ReadDouble(
      table, "margin_right_cm", out->margin_right_cm);
}

void FormatterConfigPayload::FillTypstStyle(const FontConfig& fonts,
                                            const LayoutConfig& layout,
                                            TypstStyleOwned* out) {
  if (out == nullptr) {
    return;
  }

  out->base_font = fonts.base_font;
  out->title_font =
      fonts.title_font.empty() ? fonts.base_font : fonts.title_font;
  out->category_title_font = fonts.category_title_font.empty()
                                 ? fonts.base_font
                                 : fonts.category_title_font;
  out->base_font_size = fonts.base_font_size;
  out->report_title_font_size = fonts.report_title_font_size;
  out->category_title_font_size = fonts.category_title_font_size;
  out->line_spacing_em = layout.line_spacing_em;
  out->margin_top_cm = layout.margin_top_cm;
  out->margin_bottom_cm = layout.margin_bottom_cm;
  out->margin_left_cm = layout.margin_left_cm;
  out->margin_right_cm = layout.margin_right_cm;
}
