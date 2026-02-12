// infrastructure/reports/monthly/formatters/typst/month_typ_config.cpp
#include "infrastructure/reports/monthly/formatters/typst/month_typ_config.hpp"

MonthTypConfig::MonthTypConfig(const TtMonthTypConfigV1& config)
    : MonthBaseConfig(config.labels),
      style_(config.style),
      margin_top_cm_(config.style.marginTopCm),
      margin_bottom_cm_(config.style.marginBottomCm),
      margin_left_cm_(config.style.marginLeftCm),
      margin_right_cm_(config.style.marginRightCm) {}

auto MonthTypConfig::GetMarginTopCm() const -> double {
  return margin_top_cm_;
}
auto MonthTypConfig::GetMarginBottomCm() const -> double {
  return margin_bottom_cm_;
}
auto MonthTypConfig::GetMarginLeftCm() const -> double {
  return margin_left_cm_;
}
auto MonthTypConfig::GetMarginRightCm() const -> double {
  return margin_right_cm_;
}
