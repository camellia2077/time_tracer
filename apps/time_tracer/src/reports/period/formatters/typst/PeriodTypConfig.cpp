// reports/period/formatters/typst/PeriodTypConfig.cpp
#include "PeriodTypConfig.hpp"

PeriodTypConfig::PeriodTypConfig(const std::filesystem::path& config_path)
    : PeriodBaseConfig(config_path),
      style_(config_json_)
{
    margin_top_cm_ = config_json_.at("margin_top_cm").get<double>();
    margin_bottom_cm_ = config_json_.at("margin_bottom_cm").get<double>();
    margin_left_cm_ = config_json_.at("margin_left_cm").get<double>();
    margin_right_cm_ = config_json_.at("margin_right_cm").get<double>();
}

double PeriodTypConfig::get_margin_top_cm() const { return margin_top_cm_; }
double PeriodTypConfig::get_margin_bottom_cm() const { return margin_bottom_cm_; }
double PeriodTypConfig::get_margin_left_cm() const { return margin_left_cm_; }
double PeriodTypConfig::get_margin_right_cm() const { return margin_right_cm_; }