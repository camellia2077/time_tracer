// infrastructure/reports/shared/factories/formatter_config_payload.hpp
#ifndef REPORTS_SHARED_FACTORIES_FORMATTER_CONFIG_PAYLOAD_H_
#define REPORTS_SHARED_FACTORIES_FORMATTER_CONFIG_PAYLOAD_H_

#include <toml++/toml.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "infrastructure/config/models/report_config_models.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

namespace formatter_config_payload_detail {

inline auto ToStringView(const std::string& value) -> TtStringView {
  TtStringView view{};
  if (!value.empty()) {
    view.data = value.data();
  }
  view.length = static_cast<uint64_t>(value.size());
  return view;
}

inline auto ReadString(const toml::table& table, const char* key,
                       std::string fallback) -> std::string {
  if (auto value = table[key].value<std::string>(); value.has_value()) {
    return *value;
  }
  return fallback;
}

inline auto ReadInt32(const toml::table& table, const char* key,
                      int32_t fallback) -> int32_t {
  if (auto value = table[key].value<int64_t>(); value.has_value()) {
    if ((*value < static_cast<int64_t>(std::numeric_limits<int32_t>::min())) ||
        (*value > static_cast<int64_t>(std::numeric_limits<int32_t>::max()))) {
      throw std::overflow_error(std::string("Out-of-range int32 config key: ") +
                                key);
    }
    return static_cast<int32_t>(*value);
  }
  return fallback;
}

inline auto ReadDouble(const toml::table& table, const char* key,
                       double fallback) -> double {
  if (auto value = table[key].value<double>(); value.has_value()) {
    return *value;
  }
  return fallback;
}

inline auto ToUint32Count(std::size_t value, const char* label) -> uint32_t {
  if (value > static_cast<std::size_t>(std::numeric_limits<uint32_t>::max())) {
    throw std::overflow_error(std::string("Too many elements in ") + label +
                              ".");
  }
  return static_cast<uint32_t>(value);
}

}  // namespace formatter_config_payload_detail

class FormatterConfigPayload {
 public:
  FormatterConfigPayload();

  void BuildFromToml(uint32_t config_kind, const toml::table& config_table);
  void BuildFromLoadedDailyMdConfig(const DailyMdConfig& config);
  void BuildFromLoadedDailyTexConfig(const DailyTexConfig& config);
  void BuildFromLoadedDailyTypConfig(const DailyTypConfig& config);
  void BuildFromLoadedMonthMdConfig(const MonthlyMdConfig& config);
  void BuildFromLoadedMonthTexConfig(const MonthlyTexConfig& config);
  void BuildFromLoadedMonthTypConfig(const MonthlyTypConfig& config);
  void BuildFromLoadedRangeMdConfig(const RangeReportLabels& labels);
  void BuildFromLoadedRangeTexConfig(const RangeReportLabels& labels,
                                     const FontConfig& fonts,
                                     const LayoutConfig& layout);
  void BuildFromLoadedRangeTypConfig(const RangeReportLabels& labels,
                                     const FontConfig& fonts,
                                     const LayoutConfig& layout);

  [[nodiscard]] auto GetCConfig() const -> const TtFormatterConfig&;

 private:
  struct KeywordColorOwned {
    std::string key;
    std::string color;
  };

  struct StatisticItemNodeOwned {
    std::string label;
    std::string db_column;
    uint8_t show = 1U;
    int32_t parent_index = -1;
  };

  struct DayLabelsOwned {
    std::string title_prefix;
    std::string report_title;
    std::string date_label;
    std::string total_time_label;
    std::string status_label;
    std::string sleep_label;
    std::string getup_time_label;
    std::string remark_label;
    std::string exercise_label;
    std::string no_records_message;
    std::string statistics_label;
    std::string all_activities_label;
    std::string activity_remark_label;
    std::string activity_connector;
    std::string project_breakdown_label;
  };

  struct MonthLabelsOwned {
    std::string report_title;
    std::string title_template;
    std::string actual_days_label;
    std::string status_days_label;
    std::string sleep_days_label;
    std::string exercise_days_label;
    std::string cardio_days_label;
    std::string anaerobic_days_label;
    std::string total_time_label;
    std::string no_records_message;
    std::string invalid_format_message;
    std::string project_breakdown_label;
  };

  struct RangeLabelsOwned {
    std::string title_template;
    std::string actual_days_label;
    std::string status_days_label;
    std::string sleep_days_label;
    std::string exercise_days_label;
    std::string cardio_days_label;
    std::string anaerobic_days_label;
    std::string total_time_label;
    std::string no_records_message;
    std::string invalid_range_message;
    std::string project_breakdown_label;
  };

  struct TexStyleOwned {
    static constexpr int32_t kDefaultBaseFontSize = 10;
    static constexpr int32_t kDefaultReportTitleFontSize = 14;
    static constexpr int32_t kDefaultCategoryTitleFontSize = 12;
    static constexpr double kDefaultMarginIn = 1.0;
    static constexpr double kDefaultListTopSepPt = 0.0;
    static constexpr double kDefaultListItemSepEx = 0.0;

    std::string main_font;
    std::string cjk_main_font;
    int32_t base_font_size = kDefaultBaseFontSize;
    int32_t report_title_font_size = kDefaultReportTitleFontSize;
    int32_t category_title_font_size = kDefaultCategoryTitleFontSize;
    double margin_in = kDefaultMarginIn;
    double list_top_sep_pt = kDefaultListTopSepPt;
    double list_item_sep_ex = kDefaultListItemSepEx;
  };

  struct TypstStyleOwned {
    static constexpr int32_t kDefaultBaseFontSize = 10;
    static constexpr int32_t kDefaultReportTitleFontSize = 14;
    static constexpr int32_t kDefaultCategoryTitleFontSize = 12;
    static constexpr double kDefaultLineSpacingEm = 0.65;
    static constexpr double kDefaultMarginTopCm = 2.5;
    static constexpr double kDefaultMarginBottomCm = 2.5;
    static constexpr double kDefaultMarginLeftCm = 2.0;
    static constexpr double kDefaultMarginRightCm = 2.0;

    std::string base_font;
    std::string title_font;
    std::string category_title_font;
    int32_t base_font_size = kDefaultBaseFontSize;
    int32_t report_title_font_size = kDefaultReportTitleFontSize;
    int32_t category_title_font_size = kDefaultCategoryTitleFontSize;
    double line_spacing_em = kDefaultLineSpacingEm;
    double margin_top_cm = kDefaultMarginTopCm;
    double margin_bottom_cm = kDefaultMarginBottomCm;
    double margin_left_cm = kDefaultMarginLeftCm;
    double margin_right_cm = kDefaultMarginRightCm;
  };

  static void FlattenStatisticsItems(const toml::array* array,
                                     int32_t parent_index,
                                     std::vector<StatisticItemNodeOwned>* out);
  static void FillKeywordColors(const toml::table& table,
                                std::vector<KeywordColorOwned>* out);
  static void FillKeywordColors(
      const std::map<std::string, std::string>& colors,
      std::vector<KeywordColorOwned>* out);
  static void FillDayLabels(const toml::table& table, DayLabelsOwned* out);
  static void FillDayLabels(const DailyReportLabels& labels,
                            DayLabelsOwned* out);
  static void FillMonthLabels(const toml::table& table, MonthLabelsOwned* out);
  static void FillMonthLabels(const MonthlyReportLabels& labels,
                              MonthLabelsOwned* out);
  static void FillRangeLabels(const toml::table& table, RangeLabelsOwned* out);
  static void FillRangeLabels(const RangeReportLabels& labels,
                              RangeLabelsOwned* out);
  static void FillTexStyle(const toml::table& table, TexStyleOwned* out);
  static void FillTexStyle(const FontConfig& fonts, const LayoutConfig& layout,
                           TexStyleOwned* out);
  static void FillTypstStyle(const toml::table& table, TypstStyleOwned* out);
  static void FillTypstStyle(const FontConfig& fonts,
                             const LayoutConfig& layout, TypstStyleOwned* out);
  static void FlattenStatisticsItems(
      const std::vector<ReportStatisticsItem>& items, int32_t parent_index,
      std::vector<StatisticItemNodeOwned>* out);

  void BuildDayMdConfig(const toml::table& config_table);
  void BuildDayTexConfig(const toml::table& config_table);
  void BuildDayTypConfig(const toml::table& config_table);
  void BuildMonthMdConfig(const toml::table& config_table);
  void BuildMonthTexConfig(const toml::table& config_table);
  void BuildMonthTypConfig(const toml::table& config_table);
  void BuildRangeMdConfig(const toml::table& config_table);
  void BuildRangeTexConfig(const toml::table& config_table);
  void BuildRangeTypConfig(const toml::table& config_table);

  void RebuildKeywordColorsCView();
  void RebuildStatisticsCView();

  [[nodiscard]] auto BuildDayLabelsCView() const -> TtDayLabelsConfigV1;
  [[nodiscard]] auto BuildMonthLabelsCView() const -> TtMonthLabelsConfigV1;
  [[nodiscard]] auto BuildRangeLabelsCView() const -> TtRangeLabelsConfigV1;
  [[nodiscard]] auto BuildTexStyleCView() const -> TtTexStyleConfigV1;
  [[nodiscard]] auto BuildTypstStyleCView() const -> TtTypstStyleConfigV1;

  void RebuildConfigView(uint32_t config_kind, const void* config_data,
                         uint64_t config_size);
  void ResetToEmpty();

  std::vector<KeywordColorOwned> keyword_colors_owned_;
  std::vector<TtFormatterKeywordColorV1> keyword_colors_c_;
  std::vector<StatisticItemNodeOwned> statistics_owned_;
  std::vector<TtFormatterStatisticItemNodeV1> statistics_c_;

  DayLabelsOwned day_labels_owned_;
  MonthLabelsOwned month_labels_owned_;
  RangeLabelsOwned range_labels_owned_;
  TexStyleOwned tex_style_owned_;
  TypstStyleOwned typst_style_owned_;

  TtDayMdConfigV1 day_md_config_{};
  TtDayTexConfigV1 day_tex_config_{};
  TtDayTypConfigV1 day_typ_config_{};
  TtMonthMdConfigV1 month_md_config_{};
  TtMonthTexConfigV1 month_tex_config_{};
  TtMonthTypConfigV1 month_typ_config_{};
  TtRangeMdConfigV1 range_md_config_{};
  TtRangeTexConfigV1 range_tex_config_{};
  TtRangeTypConfigV1 range_typ_config_{};

  TtFormatterConfig c_config_{};
};

#endif  // REPORTS_SHARED_FACTORIES_FORMATTER_CONFIG_PAYLOAD_H_
