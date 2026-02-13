#include "infrastructure/reports/shared/factories/formatter_config_payload.hpp"

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
