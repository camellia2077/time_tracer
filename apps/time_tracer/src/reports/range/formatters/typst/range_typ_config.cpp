// reports/range/formatters/typst/range_typ_config.cpp
#include "range_typ_config.hpp"

namespace {
constexpr double kDefaultMarginTopCm = 2.5;
constexpr double kDefaultMarginBottomCm = 2.5;
constexpr double kDefaultMarginLeftCm = 2.0;
constexpr double kDefaultMarginRightCm = 2.0;
}  // namespace

RangeTypConfig::RangeTypConfig(const toml::table& config)
    : RangeBaseConfig(config),
      style_(config) {
    margin_top_cm_ = config_table_["margin_top_cm"].value_or<double>(
        static_cast<double>(kDefaultMarginTopCm));
    margin_bottom_cm_ = config_table_["margin_bottom_cm"].value_or<double>(
        static_cast<double>(kDefaultMarginBottomCm));
    margin_left_cm_ = config_table_["margin_left_cm"].value_or<double>(
        static_cast<double>(kDefaultMarginLeftCm));
    margin_right_cm_ = config_table_["margin_right_cm"].value_or<double>(
        static_cast<double>(kDefaultMarginRightCm));
}

auto RangeTypConfig::get_margin_top_cm() const -> double {
    return margin_top_cm_;
}

auto RangeTypConfig::get_margin_bottom_cm() const -> double {
    return margin_bottom_cm_;
}

auto RangeTypConfig::get_margin_left_cm() const -> double {
    return margin_left_cm_;
}

auto RangeTypConfig::get_margin_right_cm() const -> double {
    return margin_right_cm_;
}
