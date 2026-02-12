// infrastructure/reports/range/formatters/typst/range_typ_config.cpp
#include "infrastructure/reports/range/formatters/typst/range_typ_config.hpp"

RangeTypConfig::RangeTypConfig(const TtRangeTypConfigV1& config)
    : RangeBaseConfig(config.labels),
      style_(config.style),
      margin_top_cm_(config.style.marginTopCm),
      margin_bottom_cm_(config.style.marginBottomCm),
      margin_left_cm_(config.style.marginLeftCm),
      margin_right_cm_(config.style.marginRightCm) {}

auto RangeTypConfig::GetMarginTopCm() const -> double {
  return margin_top_cm_;
}

auto RangeTypConfig::GetMarginBottomCm() const -> double {
  return margin_bottom_cm_;
}

auto RangeTypConfig::GetMarginLeftCm() const -> double {
  return margin_left_cm_;
}

auto RangeTypConfig::GetMarginRightCm() const -> double {
  return margin_right_cm_;
}
