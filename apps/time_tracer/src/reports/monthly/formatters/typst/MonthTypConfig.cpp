// reports/monthly/formatters/typst/MonthTypConfig.cpp
#include "MonthTypConfig.hpp"

MonthTypConfig::MonthTypConfig(const std::filesystem::path& config_path)
    : MonthBaseConfig(config_path),
      style_(config_json_)
{
    margin_top_cm_ = config_json_.at("margin_top_cm").get<double>();
    margin_bottom_cm_ = config_json_.at("margin_bottom_cm").get<double>();
    margin_left_cm_ = config_json_.at("margin_left_cm").get<double>();
    margin_right_cm_ = config_json_.at("margin_right_cm").get<double>();
}

double MonthTypConfig::get_margin_top_cm() const { return margin_top_cm_; }
double MonthTypConfig::get_margin_bottom_cm() const { return margin_bottom_cm_; }
double MonthTypConfig::get_margin_left_cm() const { return margin_left_cm_; }
double MonthTypConfig::get_margin_right_cm() const { return margin_right_cm_; }