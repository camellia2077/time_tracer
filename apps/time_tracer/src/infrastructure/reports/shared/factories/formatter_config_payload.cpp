#include "infrastructure/reports/shared/factories/formatter_config_payload.hpp"

#include <utility>

namespace {
constexpr int32_t kDefaultStatisticFontSize = 10;
constexpr int32_t kDefaultStatisticTitleFontSize = 12;
}  // namespace

FormatterConfigPayload::FormatterConfigPayload() {
  ResetToEmpty();
}

void FormatterConfigPayload::BuildFromToml(uint32_t config_kind,
                                           const toml::table& config_table) {
  ResetToEmpty();
  switch (config_kind) {
    case TT_FORMATTER_CONFIG_KIND_DAY_MD:
      BuildDayMdConfig(config_table);
      break;
    case TT_FORMATTER_CONFIG_KIND_DAY_TEX:
      BuildDayTexConfig(config_table);
      break;
    case TT_FORMATTER_CONFIG_KIND_DAY_TYP:
      BuildDayTypConfig(config_table);
      break;
    case TT_FORMATTER_CONFIG_KIND_MONTH_MD:
      BuildMonthMdConfig(config_table);
      break;
    case TT_FORMATTER_CONFIG_KIND_MONTH_TEX:
      BuildMonthTexConfig(config_table);
      break;
    case TT_FORMATTER_CONFIG_KIND_MONTH_TYP:
      BuildMonthTypConfig(config_table);
      break;
    case TT_FORMATTER_CONFIG_KIND_RANGE_MD:
      BuildRangeMdConfig(config_table);
      break;
    case TT_FORMATTER_CONFIG_KIND_RANGE_TEX:
      BuildRangeTexConfig(config_table);
      break;
    case TT_FORMATTER_CONFIG_KIND_RANGE_TYP:
      BuildRangeTypConfig(config_table);
      break;
    default:
      RebuildConfigView(config_kind, nullptr, 0U);
      break;
  }
}

void FormatterConfigPayload::BuildFromLoadedDailyMdConfig(
    const DailyMdConfig& config) {
  ResetToEmpty();
  FillDayLabels(config.labels, &day_labels_owned_);
  statistics_owned_.clear();
  FlattenStatisticsItems(config.statistics_items, -1, &statistics_owned_);
  RebuildStatisticsCView();

  day_md_config_.structSize = static_cast<uint32_t>(sizeof(TtDayMdConfigV1));
  day_md_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  day_md_config_.labels = BuildDayLabelsCView();
  day_md_config_.statisticsItems =
      statistics_c_.empty() ? nullptr : statistics_c_.data();
  day_md_config_.statisticsItemCount =
      formatter_config_payload_detail::ToUint32Count(statistics_c_.size(),
                                                     "day_md.statistics");
  day_md_config_.reserved = 0U;

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_DAY_MD, &day_md_config_,
                    sizeof(TtDayMdConfigV1));
}

void FormatterConfigPayload::BuildFromLoadedDailyTexConfig(
    const DailyTexConfig& config) {
  ResetToEmpty();
  FillDayLabels(config.labels, &day_labels_owned_);
  FillTexStyle(config.fonts, config.layout, &tex_style_owned_);
  FillKeywordColors(config.keyword_colors, &keyword_colors_owned_);
  statistics_owned_.clear();
  FlattenStatisticsItems(config.statistics_items, -1, &statistics_owned_);
  RebuildKeywordColorsCView();
  RebuildStatisticsCView();

  day_tex_config_.structSize = static_cast<uint32_t>(sizeof(TtDayTexConfigV1));
  day_tex_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  day_tex_config_.labels = BuildDayLabelsCView();
  day_tex_config_.style = BuildTexStyleCView();
  day_tex_config_.keywordColors =
      keyword_colors_c_.empty() ? nullptr : keyword_colors_c_.data();
  day_tex_config_.keywordColorCount =
      formatter_config_payload_detail::ToUint32Count(keyword_colors_c_.size(),
                                                     "day_tex.keyword_colors");
  day_tex_config_.statisticsItems =
      statistics_c_.empty() ? nullptr : statistics_c_.data();
  day_tex_config_.statisticsItemCount =
      formatter_config_payload_detail::ToUint32Count(statistics_c_.size(),
                                                     "day_tex.statistics");

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_DAY_TEX, &day_tex_config_,
                    sizeof(TtDayTexConfigV1));
}

void FormatterConfigPayload::BuildFromLoadedDailyTypConfig(
    const DailyTypConfig& config) {
  ResetToEmpty();
  FillDayLabels(config.labels, &day_labels_owned_);
  FillTypstStyle(config.fonts, config.layout, &typst_style_owned_);
  FillKeywordColors(config.keyword_colors, &keyword_colors_owned_);
  statistics_owned_.clear();
  FlattenStatisticsItems(config.statistics_items, -1, &statistics_owned_);
  RebuildKeywordColorsCView();
  RebuildStatisticsCView();

  day_typ_config_.structSize = static_cast<uint32_t>(sizeof(TtDayTypConfigV1));
  day_typ_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  day_typ_config_.labels = BuildDayLabelsCView();
  day_typ_config_.style = BuildTypstStyleCView();
  day_typ_config_.keywordColors =
      keyword_colors_c_.empty() ? nullptr : keyword_colors_c_.data();
  day_typ_config_.keywordColorCount =
      formatter_config_payload_detail::ToUint32Count(keyword_colors_c_.size(),
                                                     "day_typ.keyword_colors");
  day_typ_config_.statisticsItems =
      statistics_c_.empty() ? nullptr : statistics_c_.data();
  day_typ_config_.statisticsItemCount =
      formatter_config_payload_detail::ToUint32Count(statistics_c_.size(),
                                                     "day_typ.statistics");
  day_typ_config_.statisticFontSize = config.statistic_font_size;
  day_typ_config_.statisticTitleFontSize = config.statistic_title_font_size;

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_DAY_TYP, &day_typ_config_,
                    sizeof(TtDayTypConfigV1));
}

void FormatterConfigPayload::BuildFromLoadedMonthMdConfig(
    const MonthlyMdConfig& config) {
  ResetToEmpty();
  FillMonthLabels(config.labels, &month_labels_owned_);

  month_md_config_.structSize =
      static_cast<uint32_t>(sizeof(TtMonthMdConfigV1));
  month_md_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  month_md_config_.labels = BuildMonthLabelsCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_MONTH_MD, &month_md_config_,
                    sizeof(TtMonthMdConfigV1));
}

void FormatterConfigPayload::BuildFromLoadedMonthTexConfig(
    const MonthlyTexConfig& config) {
  ResetToEmpty();
  FillMonthLabels(config.labels, &month_labels_owned_);
  FillTexStyle(config.fonts, config.layout, &tex_style_owned_);

  month_tex_config_.structSize =
      static_cast<uint32_t>(sizeof(TtMonthTexConfigV1));
  month_tex_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  month_tex_config_.labels = BuildMonthLabelsCView();
  month_tex_config_.style = BuildTexStyleCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_MONTH_TEX, &month_tex_config_,
                    sizeof(TtMonthTexConfigV1));
}

void FormatterConfigPayload::BuildFromLoadedMonthTypConfig(
    const MonthlyTypConfig& config) {
  ResetToEmpty();
  FillMonthLabels(config.labels, &month_labels_owned_);
  FillTypstStyle(config.fonts, config.layout, &typst_style_owned_);

  month_typ_config_.structSize =
      static_cast<uint32_t>(sizeof(TtMonthTypConfigV1));
  month_typ_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  month_typ_config_.labels = BuildMonthLabelsCView();
  month_typ_config_.style = BuildTypstStyleCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_MONTH_TYP, &month_typ_config_,
                    sizeof(TtMonthTypConfigV1));
}

void FormatterConfigPayload::BuildFromLoadedRangeMdConfig(
    const RangeReportLabels& labels) {
  ResetToEmpty();
  FillRangeLabels(labels, &range_labels_owned_);

  range_md_config_.structSize =
      static_cast<uint32_t>(sizeof(TtRangeMdConfigV1));
  range_md_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  range_md_config_.labels = BuildRangeLabelsCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_RANGE_MD, &range_md_config_,
                    sizeof(TtRangeMdConfigV1));
}

void FormatterConfigPayload::BuildFromLoadedRangeTexConfig(
    const RangeReportLabels& labels, const FontConfig& fonts,
    const LayoutConfig& layout) {
  ResetToEmpty();
  FillRangeLabels(labels, &range_labels_owned_);
  FillTexStyle(fonts, layout, &tex_style_owned_);

  range_tex_config_.structSize =
      static_cast<uint32_t>(sizeof(TtRangeTexConfigV1));
  range_tex_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  range_tex_config_.labels = BuildRangeLabelsCView();
  range_tex_config_.style = BuildTexStyleCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_RANGE_TEX, &range_tex_config_,
                    sizeof(TtRangeTexConfigV1));
}

void FormatterConfigPayload::BuildFromLoadedRangeTypConfig(
    const RangeReportLabels& labels, const FontConfig& fonts,
    const LayoutConfig& layout) {
  ResetToEmpty();
  FillRangeLabels(labels, &range_labels_owned_);
  FillTypstStyle(fonts, layout, &typst_style_owned_);

  range_typ_config_.structSize =
      static_cast<uint32_t>(sizeof(TtRangeTypConfigV1));
  range_typ_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  range_typ_config_.labels = BuildRangeLabelsCView();
  range_typ_config_.style = BuildTypstStyleCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_RANGE_TYP, &range_typ_config_,
                    sizeof(TtRangeTypConfigV1));
}

auto FormatterConfigPayload::GetCConfig() const -> const TtFormatterConfig& {
  return c_config_;
}

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
  out->date_label = labels.date_label;
  out->total_time_label = labels.total_time_label;
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

void FormatterConfigPayload::BuildDayMdConfig(const toml::table& config_table) {
  FillDayLabels(config_table, &day_labels_owned_);
  statistics_owned_.clear();
  FlattenStatisticsItems(config_table["statistics_items"].as_array(), -1,
                         &statistics_owned_);
  RebuildStatisticsCView();

  day_md_config_.structSize = static_cast<uint32_t>(sizeof(TtDayMdConfigV1));
  day_md_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  day_md_config_.labels = BuildDayLabelsCView();
  day_md_config_.statisticsItems =
      statistics_c_.empty() ? nullptr : statistics_c_.data();
  day_md_config_.statisticsItemCount =
      formatter_config_payload_detail::ToUint32Count(statistics_c_.size(),
                                                     "day_md.statistics");
  day_md_config_.reserved = 0U;

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_DAY_MD, &day_md_config_,
                    sizeof(TtDayMdConfigV1));
}

void FormatterConfigPayload::BuildDayTexConfig(
    const toml::table& config_table) {
  FillDayLabels(config_table, &day_labels_owned_);
  FillTexStyle(config_table, &tex_style_owned_);
  FillKeywordColors(config_table, &keyword_colors_owned_);

  statistics_owned_.clear();
  FlattenStatisticsItems(config_table["statistics_items"].as_array(), -1,
                         &statistics_owned_);

  RebuildKeywordColorsCView();
  RebuildStatisticsCView();

  day_tex_config_.structSize = static_cast<uint32_t>(sizeof(TtDayTexConfigV1));
  day_tex_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  day_tex_config_.labels = BuildDayLabelsCView();
  day_tex_config_.style = BuildTexStyleCView();
  day_tex_config_.keywordColors =
      keyword_colors_c_.empty() ? nullptr : keyword_colors_c_.data();
  day_tex_config_.keywordColorCount =
      formatter_config_payload_detail::ToUint32Count(keyword_colors_c_.size(),
                                                     "day_tex.keyword_colors");
  day_tex_config_.statisticsItems =
      statistics_c_.empty() ? nullptr : statistics_c_.data();
  day_tex_config_.statisticsItemCount =
      formatter_config_payload_detail::ToUint32Count(statistics_c_.size(),
                                                     "day_tex.statistics");

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_DAY_TEX, &day_tex_config_,
                    sizeof(TtDayTexConfigV1));
}

void FormatterConfigPayload::BuildDayTypConfig(
    const toml::table& config_table) {
  FillDayLabels(config_table, &day_labels_owned_);
  FillTypstStyle(config_table, &typst_style_owned_);
  FillKeywordColors(config_table, &keyword_colors_owned_);

  statistics_owned_.clear();
  FlattenStatisticsItems(config_table["statistics_items"].as_array(), -1,
                         &statistics_owned_);

  RebuildKeywordColorsCView();
  RebuildStatisticsCView();

  day_typ_config_.structSize = static_cast<uint32_t>(sizeof(TtDayTypConfigV1));
  day_typ_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  day_typ_config_.labels = BuildDayLabelsCView();
  day_typ_config_.style = BuildTypstStyleCView();
  day_typ_config_.keywordColors =
      keyword_colors_c_.empty() ? nullptr : keyword_colors_c_.data();
  day_typ_config_.keywordColorCount =
      formatter_config_payload_detail::ToUint32Count(keyword_colors_c_.size(),
                                                     "day_typ.keyword_colors");
  day_typ_config_.statisticsItems =
      statistics_c_.empty() ? nullptr : statistics_c_.data();
  day_typ_config_.statisticsItemCount =
      formatter_config_payload_detail::ToUint32Count(statistics_c_.size(),
                                                     "day_typ.statistics");
  day_typ_config_.statisticFontSize =
      formatter_config_payload_detail::ReadInt32(
          config_table, "statistic_font_size", kDefaultStatisticFontSize);
  day_typ_config_.statisticTitleFontSize =
      formatter_config_payload_detail::ReadInt32(
          config_table, "statistic_title_font_size",
          kDefaultStatisticTitleFontSize);

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_DAY_TYP, &day_typ_config_,
                    sizeof(TtDayTypConfigV1));
}

void FormatterConfigPayload::BuildMonthMdConfig(
    const toml::table& config_table) {
  FillMonthLabels(config_table, &month_labels_owned_);

  month_md_config_.structSize =
      static_cast<uint32_t>(sizeof(TtMonthMdConfigV1));
  month_md_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  month_md_config_.labels = BuildMonthLabelsCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_MONTH_MD, &month_md_config_,
                    sizeof(TtMonthMdConfigV1));
}

void FormatterConfigPayload::BuildMonthTexConfig(
    const toml::table& config_table) {
  FillMonthLabels(config_table, &month_labels_owned_);
  FillTexStyle(config_table, &tex_style_owned_);

  month_tex_config_.structSize =
      static_cast<uint32_t>(sizeof(TtMonthTexConfigV1));
  month_tex_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  month_tex_config_.labels = BuildMonthLabelsCView();
  month_tex_config_.style = BuildTexStyleCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_MONTH_TEX, &month_tex_config_,
                    sizeof(TtMonthTexConfigV1));
}

void FormatterConfigPayload::BuildMonthTypConfig(
    const toml::table& config_table) {
  FillMonthLabels(config_table, &month_labels_owned_);
  FillTypstStyle(config_table, &typst_style_owned_);

  month_typ_config_.structSize =
      static_cast<uint32_t>(sizeof(TtMonthTypConfigV1));
  month_typ_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  month_typ_config_.labels = BuildMonthLabelsCView();
  month_typ_config_.style = BuildTypstStyleCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_MONTH_TYP, &month_typ_config_,
                    sizeof(TtMonthTypConfigV1));
}

void FormatterConfigPayload::BuildRangeMdConfig(
    const toml::table& config_table) {
  FillRangeLabels(config_table, &range_labels_owned_);

  range_md_config_.structSize =
      static_cast<uint32_t>(sizeof(TtRangeMdConfigV1));
  range_md_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  range_md_config_.labels = BuildRangeLabelsCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_RANGE_MD, &range_md_config_,
                    sizeof(TtRangeMdConfigV1));
}

void FormatterConfigPayload::BuildRangeTexConfig(
    const toml::table& config_table) {
  FillRangeLabels(config_table, &range_labels_owned_);
  FillTexStyle(config_table, &tex_style_owned_);

  range_tex_config_.structSize =
      static_cast<uint32_t>(sizeof(TtRangeTexConfigV1));
  range_tex_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  range_tex_config_.labels = BuildRangeLabelsCView();
  range_tex_config_.style = BuildTexStyleCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_RANGE_TEX, &range_tex_config_,
                    sizeof(TtRangeTexConfigV1));
}

void FormatterConfigPayload::BuildRangeTypConfig(
    const toml::table& config_table) {
  FillRangeLabels(config_table, &range_labels_owned_);
  FillTypstStyle(config_table, &typst_style_owned_);

  range_typ_config_.structSize =
      static_cast<uint32_t>(sizeof(TtRangeTypConfigV1));
  range_typ_config_.version = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  range_typ_config_.labels = BuildRangeLabelsCView();
  range_typ_config_.style = BuildTypstStyleCView();

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_RANGE_TYP, &range_typ_config_,
                    sizeof(TtRangeTypConfigV1));
}

void FormatterConfigPayload::RebuildKeywordColorsCView() {
  keyword_colors_c_.clear();
  keyword_colors_c_.reserve(keyword_colors_owned_.size());

  for (const auto& item : keyword_colors_owned_) {
    TtFormatterKeywordColorV1 c_item{};
    c_item.keyword = formatter_config_payload_detail::ToStringView(item.key);
    c_item.color = formatter_config_payload_detail::ToStringView(item.color);
    keyword_colors_c_.push_back(c_item);
  }
}

void FormatterConfigPayload::RebuildStatisticsCView() {
  statistics_c_.clear();
  statistics_c_.reserve(statistics_owned_.size());

  for (const auto& item : statistics_owned_) {
    TtFormatterStatisticItemNodeV1 c_item{};
    c_item.label = formatter_config_payload_detail::ToStringView(item.label);
    c_item.dbColumn =
        formatter_config_payload_detail::ToStringView(item.db_column);
    c_item.show = item.show;
    std::ranges::fill(c_item.reserved0, 0U);
    c_item.parentIndex = item.parent_index;
    statistics_c_.push_back(c_item);
  }
}

auto FormatterConfigPayload::BuildDayLabelsCView() const
    -> TtDayLabelsConfigV1 {
  TtDayLabelsConfigV1 labels{};
  labels.titlePrefix = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.title_prefix);
  labels.reportTitle = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.report_title);
  labels.dateLabel = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.date_label);
  labels.totalTimeLabel = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.total_time_label);
  labels.statusLabel = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.status_label);
  labels.sleepLabel = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.sleep_label);
  labels.getupTimeLabel = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.getup_time_label);
  labels.remarkLabel = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.remark_label);
  labels.exerciseLabel = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.exercise_label);
  labels.noRecordsMessage = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.no_records_message);
  labels.statisticsLabel = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.statistics_label);
  labels.allActivitiesLabel = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.all_activities_label);
  labels.activityRemarkLabel = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.activity_remark_label);
  labels.activityConnector = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.activity_connector);
  labels.projectBreakdownLabel = formatter_config_payload_detail::ToStringView(
      day_labels_owned_.project_breakdown_label);
  return labels;
}

auto FormatterConfigPayload::BuildMonthLabelsCView() const
    -> TtMonthLabelsConfigV1 {
  TtMonthLabelsConfigV1 labels{};
  labels.reportTitle = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.report_title);
  labels.titleTemplate = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.title_template);
  labels.actualDaysLabel = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.actual_days_label);
  labels.statusDaysLabel = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.status_days_label);
  labels.sleepDaysLabel = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.sleep_days_label);
  labels.exerciseDaysLabel = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.exercise_days_label);
  labels.cardioDaysLabel = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.cardio_days_label);
  labels.anaerobicDaysLabel = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.anaerobic_days_label);
  labels.totalTimeLabel = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.total_time_label);
  labels.noRecordsMessage = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.no_records_message);
  labels.invalidFormatMessage = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.invalid_format_message);
  labels.projectBreakdownLabel = formatter_config_payload_detail::ToStringView(
      month_labels_owned_.project_breakdown_label);
  return labels;
}

auto FormatterConfigPayload::BuildRangeLabelsCView() const
    -> TtRangeLabelsConfigV1 {
  TtRangeLabelsConfigV1 labels{};
  labels.titleTemplate = formatter_config_payload_detail::ToStringView(
      range_labels_owned_.title_template);
  labels.actualDaysLabel = formatter_config_payload_detail::ToStringView(
      range_labels_owned_.actual_days_label);
  labels.statusDaysLabel = formatter_config_payload_detail::ToStringView(
      range_labels_owned_.status_days_label);
  labels.sleepDaysLabel = formatter_config_payload_detail::ToStringView(
      range_labels_owned_.sleep_days_label);
  labels.exerciseDaysLabel = formatter_config_payload_detail::ToStringView(
      range_labels_owned_.exercise_days_label);
  labels.cardioDaysLabel = formatter_config_payload_detail::ToStringView(
      range_labels_owned_.cardio_days_label);
  labels.anaerobicDaysLabel = formatter_config_payload_detail::ToStringView(
      range_labels_owned_.anaerobic_days_label);
  labels.totalTimeLabel = formatter_config_payload_detail::ToStringView(
      range_labels_owned_.total_time_label);
  labels.noRecordsMessage = formatter_config_payload_detail::ToStringView(
      range_labels_owned_.no_records_message);
  labels.invalidRangeMessage = formatter_config_payload_detail::ToStringView(
      range_labels_owned_.invalid_range_message);
  labels.projectBreakdownLabel = formatter_config_payload_detail::ToStringView(
      range_labels_owned_.project_breakdown_label);
  return labels;
}

auto FormatterConfigPayload::BuildTexStyleCView() const -> TtTexStyleConfigV1 {
  TtTexStyleConfigV1 style{};
  style.mainFont =
      formatter_config_payload_detail::ToStringView(tex_style_owned_.main_font);
  style.cjkMainFont = formatter_config_payload_detail::ToStringView(
      tex_style_owned_.cjk_main_font);
  style.baseFontSize = tex_style_owned_.base_font_size;
  style.reportTitleFontSize = tex_style_owned_.report_title_font_size;
  style.categoryTitleFontSize = tex_style_owned_.category_title_font_size;
  style.marginIn = tex_style_owned_.margin_in;
  style.listTopSepPt = tex_style_owned_.list_top_sep_pt;
  style.listItemSepEx = tex_style_owned_.list_item_sep_ex;
  return style;
}

auto FormatterConfigPayload::BuildTypstStyleCView() const
    -> TtTypstStyleConfigV1 {
  TtTypstStyleConfigV1 style{};
  style.baseFont = formatter_config_payload_detail::ToStringView(
      typst_style_owned_.base_font);
  style.titleFont = formatter_config_payload_detail::ToStringView(
      typst_style_owned_.title_font);
  style.categoryTitleFont = formatter_config_payload_detail::ToStringView(
      typst_style_owned_.category_title_font);
  style.baseFontSize = typst_style_owned_.base_font_size;
  style.reportTitleFontSize = typst_style_owned_.report_title_font_size;
  style.categoryTitleFontSize = typst_style_owned_.category_title_font_size;
  style.lineSpacingEm = typst_style_owned_.line_spacing_em;
  style.marginTopCm = typst_style_owned_.margin_top_cm;
  style.marginBottomCm = typst_style_owned_.margin_bottom_cm;
  style.marginLeftCm = typst_style_owned_.margin_left_cm;
  style.marginRightCm = typst_style_owned_.margin_right_cm;
  return style;
}

void FormatterConfigPayload::RebuildConfigView(uint32_t config_kind,
                                               const void* config_data,
                                               uint64_t config_size) {
  c_config_.structSize = static_cast<uint32_t>(sizeof(TtFormatterConfig));
  c_config_.version = TT_FORMATTER_CONFIG_VIEW_VERSION_V1;
  c_config_.configKind = config_kind;
  c_config_.configVersion = TT_FORMATTER_CONFIG_DATA_VERSION_V1;
  c_config_.configData = config_data;
  c_config_.configDataSize = config_size;
}

void FormatterConfigPayload::ResetToEmpty() {
  keyword_colors_owned_.clear();
  keyword_colors_c_.clear();
  statistics_owned_.clear();
  statistics_c_.clear();

  day_labels_owned_ = DayLabelsOwned{};
  month_labels_owned_ = MonthLabelsOwned{};
  range_labels_owned_ = RangeLabelsOwned{};
  tex_style_owned_ = TexStyleOwned{};
  typst_style_owned_ = TypstStyleOwned{};

  day_md_config_ = TtDayMdConfigV1{};
  day_tex_config_ = TtDayTexConfigV1{};
  day_typ_config_ = TtDayTypConfigV1{};
  month_md_config_ = TtMonthMdConfigV1{};
  month_tex_config_ = TtMonthTexConfigV1{};
  month_typ_config_ = TtMonthTypConfigV1{};
  range_md_config_ = TtRangeMdConfigV1{};
  range_tex_config_ = TtRangeTexConfigV1{};
  range_typ_config_ = TtRangeTypConfigV1{};

  RebuildConfigView(TT_FORMATTER_CONFIG_KIND_UNKNOWN, nullptr, 0U);
}
