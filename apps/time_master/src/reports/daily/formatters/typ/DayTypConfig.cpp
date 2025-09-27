// reports/daily/formatters/typ/DayTypConfig.cpp
#include "DayTypConfig.hpp"

// [修改] 构造函数调用基类构造函数，然后只加载自己的特有项
DayTypConfig::DayTypConfig(const std::string& config_path)
    : DayBaseConfig(config_path) // 调用基类构造函数
{
    // 使用基类中已加载的 config_json_ 来加载本类特有的配置项
    title_font_ = config_json_.at("title_font").get<std::string>();
    base_font_ = config_json_.at("base_font").get<std::string>();
    category_title_font_ = config_json_.at("category_title_font").get<std::string>();
    base_font_size_ = config_json_.at("base_font_size").get<int>();
    report_title_font_size_ = config_json_.at("report_title_font_size").get<int>();
    category_title_font_size_ = config_json_.at("category_title_font_size").get<int>();
    statistic_font_size_ = config_json_.at("statistic_font_size").get<int>();
    statistic_title_font_size_ = config_json_.at("statistic_title_font_size").get<int>();
    line_spacing_em_ = config_json_.at("line_spacing_em").get<double>();
    keyword_colors_ = config_json_.at("keyword_colors").get<std::map<std::string, std::string>>();
}

// --- [修改] 只实现特有的 getters ---
const std::string& DayTypConfig::get_title_font() const { return title_font_; }
const std::string& DayTypConfig::get_base_font() const { return base_font_; }
const std::string& DayTypConfig::get_category_title_font() const { return category_title_font_; }
int DayTypConfig::get_base_font_size() const { return base_font_size_; }
int DayTypConfig::get_report_title_font_size() const { return report_title_font_size_; }
int DayTypConfig::get_category_title_font_size() const { return category_title_font_size_; }
int DayTypConfig::get_statistic_font_size() const { return statistic_font_size_; }
int DayTypConfig::get_statistic_title_font_size() const { return statistic_title_font_size_; }
double DayTypConfig::get_line_spacing_em() const { return line_spacing_em_; }
const std::map<std::string, std::string>& DayTypConfig::get_keyword_colors() const { return keyword_colors_; }