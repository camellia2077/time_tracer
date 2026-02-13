#include "infrastructure/reports/shared/factories/formatter_config_payload.hpp"

namespace {
constexpr int32_t kDefaultStatisticFontSize = 10;
constexpr int32_t kDefaultStatisticTitleFontSize = 12;
}  // namespace

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
  BuildFromLoaded([&]() -> void {
    BuildWithStrategy(
        TT_FORMATTER_CONFIG_KIND_DAY_MD, &day_md_config_,
        [&]() -> void {
          FillDayLabels(config.labels, &day_labels_owned_);
          LoadStatistics(config.statistics_items);
        },
        [&]() -> void { PopulateDayMdConfig(); });
  });
}

void FormatterConfigPayload::BuildFromLoadedDailyTexConfig(
    const DailyTexConfig& config) {
  BuildFromLoaded([&]() -> void {
    BuildWithStrategy(
        TT_FORMATTER_CONFIG_KIND_DAY_TEX, &day_tex_config_,
        [&]() -> void {
          FillDayLabels(config.labels, &day_labels_owned_);
          FillTexStyle(config.fonts, config.layout, &tex_style_owned_);
          LoadKeywordColors(config.keyword_colors);
          LoadStatistics(config.statistics_items);
        },
        [&]() -> void { PopulateDayTexConfig(); });
  });
}

void FormatterConfigPayload::BuildFromLoadedDailyTypConfig(
    const DailyTypConfig& config) {
  BuildFromLoaded([&]() -> void {
    BuildWithStrategy(
        TT_FORMATTER_CONFIG_KIND_DAY_TYP, &day_typ_config_,
        [&]() -> void {
          FillDayLabels(config.labels, &day_labels_owned_);
          FillTypstStyle(config.fonts, config.layout, &typst_style_owned_);
          LoadKeywordColors(config.keyword_colors);
          LoadStatistics(config.statistics_items);
        },
        [&]() -> void {
          PopulateDayTypConfig(
              {.statistic_font_size = config.statistic_font_size,
               .statistic_title_font_size = config.statistic_title_font_size});
        });
  });
}

void FormatterConfigPayload::BuildFromLoadedMonthMdConfig(
    const MonthlyMdConfig& config) {
  BuildFromLoaded([&]() -> void {
    BuildWithStrategy(
        TT_FORMATTER_CONFIG_KIND_MONTH_MD, &month_md_config_,
        [&]() -> void { FillMonthLabels(config.labels, &month_labels_owned_); },
        [&]() -> void { PopulateMonthMdConfig(); });
  });
}

void FormatterConfigPayload::BuildFromLoadedMonthTexConfig(
    const MonthlyTexConfig& config) {
  BuildFromLoaded([&]() -> void {
    BuildWithStrategy(
        TT_FORMATTER_CONFIG_KIND_MONTH_TEX, &month_tex_config_,
        [&]() -> void {
          FillMonthLabels(config.labels, &month_labels_owned_);
          FillTexStyle(config.fonts, config.layout, &tex_style_owned_);
        },
        [&]() -> void { PopulateMonthTexConfig(); });
  });
}

void FormatterConfigPayload::BuildFromLoadedMonthTypConfig(
    const MonthlyTypConfig& config) {
  BuildFromLoaded([&]() -> void {
    BuildWithStrategy(
        TT_FORMATTER_CONFIG_KIND_MONTH_TYP, &month_typ_config_,
        [&]() -> void {
          FillMonthLabels(config.labels, &month_labels_owned_);
          FillTypstStyle(config.fonts, config.layout, &typst_style_owned_);
        },
        [&]() -> void { PopulateMonthTypConfig(); });
  });
}

void FormatterConfigPayload::BuildFromLoadedRangeMdConfig(
    const RangeReportLabels& labels) {
  BuildFromLoaded([&]() -> void {
    BuildWithStrategy(
        TT_FORMATTER_CONFIG_KIND_RANGE_MD, &range_md_config_,
        [&]() -> void { FillRangeLabels(labels, &range_labels_owned_); },
        [&]() -> void { PopulateRangeMdConfig(); });
  });
}

void FormatterConfigPayload::BuildFromLoadedRangeTexConfig(
    const RangeReportLabels& labels, const FontConfig& fonts,
    const LayoutConfig& layout) {
  BuildFromLoaded([&]() -> void {
    BuildWithStrategy(
        TT_FORMATTER_CONFIG_KIND_RANGE_TEX, &range_tex_config_,
        [&]() -> void {
          FillRangeLabels(labels, &range_labels_owned_);
          FillTexStyle(fonts, layout, &tex_style_owned_);
        },
        [&]() -> void { PopulateRangeTexConfig(); });
  });
}

void FormatterConfigPayload::BuildFromLoadedRangeTypConfig(
    const RangeReportLabels& labels, const FontConfig& fonts,
    const LayoutConfig& layout) {
  BuildFromLoaded([&]() -> void {
    BuildWithStrategy(
        TT_FORMATTER_CONFIG_KIND_RANGE_TYP, &range_typ_config_,
        [&]() -> void {
          FillRangeLabels(labels, &range_labels_owned_);
          FillTypstStyle(fonts, layout, &typst_style_owned_);
        },
        [&]() -> void { PopulateRangeTypConfig(); });
  });
}

void FormatterConfigPayload::LoadStatistics(const toml::table& config_table) {
  statistics_owned_.clear();
  FlattenStatisticsItems(config_table["statistics_items"].as_array(), -1,
                         &statistics_owned_);
  RebuildStatisticsCView();
}

void FormatterConfigPayload::LoadStatistics(
    const std::vector<ReportStatisticsItem>& items) {
  statistics_owned_.clear();
  FlattenStatisticsItems(items, -1, &statistics_owned_);
  RebuildStatisticsCView();
}

void FormatterConfigPayload::LoadKeywordColors(
    const toml::table& config_table) {
  FillKeywordColors(config_table, &keyword_colors_owned_);
  RebuildKeywordColorsCView();
}

void FormatterConfigPayload::LoadKeywordColors(
    const std::map<std::string, std::string>& colors) {
  FillKeywordColors(colors, &keyword_colors_owned_);
  RebuildKeywordColorsCView();
}

void FormatterConfigPayload::BuildDayMdConfig(const toml::table& config_table) {
  BuildWithStrategy(
      TT_FORMATTER_CONFIG_KIND_DAY_MD, &day_md_config_,
      [&]() -> void {
        FillDayLabels(config_table, &day_labels_owned_);
        LoadStatistics(config_table);
      },
      [&]() -> void { PopulateDayMdConfig(); });
}

void FormatterConfigPayload::BuildDayTexConfig(
    const toml::table& config_table) {
  BuildWithStrategy(
      TT_FORMATTER_CONFIG_KIND_DAY_TEX, &day_tex_config_,
      [&]() -> void {
        FillDayLabels(config_table, &day_labels_owned_);
        FillTexStyle(config_table, &tex_style_owned_);
        LoadKeywordColors(config_table);
        LoadStatistics(config_table);
      },
      [&]() -> void { PopulateDayTexConfig(); });
}

void FormatterConfigPayload::BuildDayTypConfig(
    const toml::table& config_table) {
  DayTypStatisticSizes statistic_sizes{
      .statistic_font_size = kDefaultStatisticFontSize,
      .statistic_title_font_size = kDefaultStatisticTitleFontSize};
  BuildWithStrategy(
      TT_FORMATTER_CONFIG_KIND_DAY_TYP, &day_typ_config_,
      [&]() -> void {
        FillDayLabels(config_table, &day_labels_owned_);
        FillTypstStyle(config_table, &typst_style_owned_);
        LoadKeywordColors(config_table);
        LoadStatistics(config_table);
        statistic_sizes.statistic_font_size =
            formatter_config_payload_detail::ReadInt32(
                config_table, "statistic_font_size", kDefaultStatisticFontSize);
        statistic_sizes.statistic_title_font_size =
            formatter_config_payload_detail::ReadInt32(
                config_table, "statistic_title_font_size",
                kDefaultStatisticTitleFontSize);
      },
      [&]() -> void { PopulateDayTypConfig(statistic_sizes); });
}

void FormatterConfigPayload::BuildMonthMdConfig(
    const toml::table& config_table) {
  BuildWithStrategy(
      TT_FORMATTER_CONFIG_KIND_MONTH_MD, &month_md_config_,
      [&]() -> void { FillMonthLabels(config_table, &month_labels_owned_); },
      [&]() -> void { PopulateMonthMdConfig(); });
}

void FormatterConfigPayload::BuildMonthTexConfig(
    const toml::table& config_table) {
  BuildWithStrategy(
      TT_FORMATTER_CONFIG_KIND_MONTH_TEX, &month_tex_config_,
      [&]() -> void {
        FillMonthLabels(config_table, &month_labels_owned_);
        FillTexStyle(config_table, &tex_style_owned_);
      },
      [&]() -> void { PopulateMonthTexConfig(); });
}

void FormatterConfigPayload::BuildMonthTypConfig(
    const toml::table& config_table) {
  BuildWithStrategy(
      TT_FORMATTER_CONFIG_KIND_MONTH_TYP, &month_typ_config_,
      [&]() -> void {
        FillMonthLabels(config_table, &month_labels_owned_);
        FillTypstStyle(config_table, &typst_style_owned_);
      },
      [&]() -> void { PopulateMonthTypConfig(); });
}

void FormatterConfigPayload::BuildRangeMdConfig(
    const toml::table& config_table) {
  BuildWithStrategy(
      TT_FORMATTER_CONFIG_KIND_RANGE_MD, &range_md_config_,
      [&]() -> void { FillRangeLabels(config_table, &range_labels_owned_); },
      [&]() -> void { PopulateRangeMdConfig(); });
}

void FormatterConfigPayload::BuildRangeTexConfig(
    const toml::table& config_table) {
  BuildWithStrategy(
      TT_FORMATTER_CONFIG_KIND_RANGE_TEX, &range_tex_config_,
      [&]() -> void {
        FillRangeLabels(config_table, &range_labels_owned_);
        FillTexStyle(config_table, &tex_style_owned_);
      },
      [&]() -> void { PopulateRangeTexConfig(); });
}

void FormatterConfigPayload::BuildRangeTypConfig(
    const toml::table& config_table) {
  BuildWithStrategy(
      TT_FORMATTER_CONFIG_KIND_RANGE_TYP, &range_typ_config_,
      [&]() -> void {
        FillRangeLabels(config_table, &range_labels_owned_);
        FillTypstStyle(config_table, &typst_style_owned_);
      },
      [&]() -> void { PopulateRangeTypConfig(); });
}

void FormatterConfigPayload::PopulateDayMdConfig() {
  InitializeConfigHeader(&day_md_config_);
  day_md_config_.labels = BuildDayLabelsCView();
  day_md_config_.statisticsItems =
      statistics_c_.empty() ? nullptr : statistics_c_.data();
  day_md_config_.statisticsItemCount =
      formatter_config_payload_detail::ToUint32Count(statistics_c_.size(),
                                                     "day_md.statistics");
  day_md_config_.reserved = 0U;
}

void FormatterConfigPayload::PopulateDayTexConfig() {
  InitializeConfigHeader(&day_tex_config_);
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
}

void FormatterConfigPayload::PopulateDayTypConfig(DayTypStatisticSizes sizes) {
  InitializeConfigHeader(&day_typ_config_);
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
  day_typ_config_.statisticFontSize = sizes.statistic_font_size;
  day_typ_config_.statisticTitleFontSize = sizes.statistic_title_font_size;
}

void FormatterConfigPayload::PopulateMonthMdConfig() {
  InitializeConfigHeader(&month_md_config_);
  month_md_config_.labels = BuildMonthLabelsCView();
}

void FormatterConfigPayload::PopulateMonthTexConfig() {
  InitializeConfigHeader(&month_tex_config_);
  month_tex_config_.labels = BuildMonthLabelsCView();
  month_tex_config_.style = BuildTexStyleCView();
}

void FormatterConfigPayload::PopulateMonthTypConfig() {
  InitializeConfigHeader(&month_typ_config_);
  month_typ_config_.labels = BuildMonthLabelsCView();
  month_typ_config_.style = BuildTypstStyleCView();
}

void FormatterConfigPayload::PopulateRangeMdConfig() {
  InitializeConfigHeader(&range_md_config_);
  range_md_config_.labels = BuildRangeLabelsCView();
}

void FormatterConfigPayload::PopulateRangeTexConfig() {
  InitializeConfigHeader(&range_tex_config_);
  range_tex_config_.labels = BuildRangeLabelsCView();
  range_tex_config_.style = BuildTexStyleCView();
}

void FormatterConfigPayload::PopulateRangeTypConfig() {
  InitializeConfigHeader(&range_typ_config_);
  range_typ_config_.labels = BuildRangeLabelsCView();
  range_typ_config_.style = BuildTypstStyleCView();
}
