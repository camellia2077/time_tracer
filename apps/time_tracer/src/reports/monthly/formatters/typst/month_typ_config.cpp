// reports/monthly/formatters/typst/MonthTypConfig.cpp
#include "month_typ_config.hpp"

MonthTypConfig::MonthTypConfig(const toml::table& config)
    : MonthBaseConfig(config),
      style_(config)
{
    margin_top_cm_ = config_table_["margin_top_cm"].value_or<double>(2.5);
    margin_bottom_cm_ = config_table_["margin_bottom_cm"].value_or<double>(2.5);
    margin_left_cm_ = config_table_["margin_left_cm"].value_or<double>(2.0);
    margin_right_cm_ = config_table_["margin_right_cm"].value_or<double>(2.0);
}

double MonthTypConfig::get_margin_top_cm() const { return margin_top_cm_; }
double MonthTypConfig::get_margin_bottom_cm() const { return margin_bottom_cm_; }
double MonthTypConfig::get_margin_left_cm() const { return margin_left_cm_; }
double MonthTypConfig::get_margin_right_cm() const { return margin_right_cm_; }