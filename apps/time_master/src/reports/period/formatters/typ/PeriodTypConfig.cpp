// reports/period/formatters/typ/PeriodTypConfig.cpp
#include "PeriodTypConfig.hpp"

PeriodTypConfig::PeriodTypConfig(const std::string& config_path)
    : PeriodBaseConfig(config_path) // 调用基类构造函数
{
    // 只加载本类特有的配置项
    base_font_ = config_json_.at("base_font").get<std::string>();
    title_font_ = config_json_.at("title_font").get<std::string>();
    category_title_font_ = config_json_.at("category_title_font").get<std::string>();
    base_font_size_ = config_json_.at("base_font_size").get<int>();
    report_title_font_size_ = config_json_.at("report_title_font_size").get<int>();
    category_title_font_size_ = config_json_.at("category_title_font_size").get<int>();
    line_spacing_em_ = config_json_.at("line_spacing_em").get<double>();
    margin_top_cm_ = config_json_.at("margin_top_cm").get<double>();
    margin_bottom_cm_ = config_json_.at("margin_bottom_cm").get<double>();
    margin_left_cm_ = config_json_.at("margin_left_cm").get<double>();
    margin_right_cm_ = config_json_.at("margin_right_cm").get<double>();
}

// --- 特有 Getters 的实现 ---
const std::string& PeriodTypConfig::get_base_font() const { return base_font_; }
const std::string& PeriodTypConfig::get_title_font() const { return title_font_; }
const std::string& PeriodTypConfig::get_category_title_font() const { return category_title_font_; }
int PeriodTypConfig::get_base_font_size() const { return base_font_size_; }
int PeriodTypConfig::get_report_title_font_size() const { return report_title_font_size_; }
int PeriodTypConfig::get_category_title_font_size() const { return category_title_font_size_; }
double PeriodTypConfig::get_line_spacing_em() const { return line_spacing_em_; }
double PeriodTypConfig::get_margin_top_cm() const { return margin_top_cm_; }
double PeriodTypConfig::get_margin_bottom_cm() const { return margin_bottom_cm_; }
double PeriodTypConfig::get_margin_left_cm() const { return margin_left_cm_; }
double PeriodTypConfig::get_margin_right_cm() const { return margin_right_cm_; }