// reports/period/formatters/typst/PeriodTypConfig.cpp
#include "PeriodTypConfig.hpp"

// [修改] 接收 toml::table
PeriodTypConfig::PeriodTypConfig(const toml::table& config)
    : PeriodBaseConfig(config),
      style_(config)
{
    margin_top_cm_ = config_table_["margin_top_cm"].value_or<double>(2.5);
    margin_bottom_cm_ = config_table_["margin_bottom_cm"].value_or<double>(2.5);
    margin_left_cm_ = config_table_["margin_left_cm"].value_or<double>(2.0);
    margin_right_cm_ = config_table_["margin_right_cm"].value_or<double>(2.0);
}

double PeriodTypConfig::get_margin_top_cm() const { return margin_top_cm_; }
double PeriodTypConfig::get_margin_bottom_cm() const { return margin_bottom_cm_; }
double PeriodTypConfig::get_margin_left_cm() const { return margin_left_cm_; }
double PeriodTypConfig::get_margin_right_cm() const { return margin_right_cm_; }