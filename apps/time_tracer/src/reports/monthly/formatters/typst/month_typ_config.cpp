// reports/monthly/formatters/typst/month_typ_config.cpp
#include "reports/monthly/formatters/typst/month_typ_config.hpp"

namespace {
constexpr double kDefaultMarginTopCm = 2.5;
constexpr double kDefaultMarginSideCm = 2.0;
constexpr double kDefaultMarginBottomCm = 2.5;
}  // namespace

MonthTypConfig::MonthTypConfig(const toml::table& config)
    : MonthBaseConfig(config), style_(config) {
  margin_top_cm_ =
      config_table_["margin_top_cm"].value_or<double>(kDefaultMarginTopCm);
  margin_bottom_cm_ =
      config_table_["margin_bottom_cm"].value_or<double>(kDefaultMarginBottomCm);
  margin_left_cm_ =
      config_table_["margin_left_cm"].value_or<double>(kDefaultMarginSideCm);
  margin_right_cm_ =
      config_table_["margin_right_cm"].value_or<double>(kDefaultMarginSideCm);
}

auto MonthTypConfig::GetMarginTopCm() const -> double { return margin_top_cm_; }
auto MonthTypConfig::GetMarginBottomCm() const -> double { return margin_bottom_cm_; }
auto MonthTypConfig::GetMarginLeftCm() const -> double { return margin_left_cm_; }
auto MonthTypConfig::GetMarginRightCm() const -> double { return margin_right_cm_; }