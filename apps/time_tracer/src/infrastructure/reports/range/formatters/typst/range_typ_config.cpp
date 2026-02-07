// infrastructure/reports/range/formatters/typst/range_typ_config.cpp
#include "infrastructure/reports/range/formatters/typst/range_typ_config.hpp"

namespace {
constexpr double kDefaultMarginTopCm = 2.5;
constexpr double kDefaultMarginBottomCm = 2.5;
constexpr double kDefaultMarginLeftCm = 2.0;
constexpr double kDefaultMarginRightCm = 2.0;
}  // namespace

RangeTypConfig::RangeTypConfig(const toml::table& config)
    : RangeBaseConfig(config), style_(config) {
  margin_top_cm_ = config_table_["margin_top_cm"].value_or<double>(
      static_cast<double>(kDefaultMarginTopCm));
  margin_bottom_cm_ = config_table_["margin_bottom_cm"].value_or<double>(
      static_cast<double>(kDefaultMarginBottomCm));
  margin_left_cm_ = config_table_["margin_left_cm"].value_or<double>(
      static_cast<double>(kDefaultMarginLeftCm));
  margin_right_cm_ = config_table_["margin_right_cm"].value_or<double>(
      static_cast<double>(kDefaultMarginRightCm));
}

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
