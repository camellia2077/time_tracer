#include "infrastructure/reports/shared/factories/formatter_config_payload.hpp"

FormatterConfigPayload::FormatterConfigPayload() {
  ResetToEmpty();
}

auto FormatterConfigPayload::GetCConfig() const -> const TtFormatterConfig& {
  return c_config_;
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
