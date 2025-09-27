// reports/period/formatters/tex/PeriodTexConfig.cpp
#include "PeriodTexConfig.hpp"

PeriodTexConfig::PeriodTexConfig(const std::string& config_path)
    : PeriodBaseConfig(config_path) // 调用基类构造函数
{
    // 只加载本类特有的配置项
    main_font_ = config_json_.at("main_font").get<std::string>();
    cjk_main_font_ = config_json_.at("cjk_main_font").get<std::string>();
    base_font_size_ = config_json_.at("base_font_size").get<int>();
    report_title_font_size_ = config_json_.at("report_title_font_size").get<int>();
    category_title_font_size_ = config_json_.at("category_title_font_size").get<int>();
    margin_in_ = config_json_.at("margin_in").get<double>();
    list_top_sep_pt_ = config_json_.at("list_top_sep_pt").get<double>();
    list_item_sep_ex_ = config_json_.at("list_item_sep_ex").get<double>();
}

// --- 特有 Getters 的实现 ---
const std::string& PeriodTexConfig::get_main_font() const { return main_font_; }
const std::string& PeriodTexConfig::get_cjk_main_font() const { return cjk_main_font_; }
int PeriodTexConfig::get_base_font_size() const { return base_font_size_; }
int PeriodTexConfig::get_report_title_font_size() const { return report_title_font_size_; }
int PeriodTexConfig::get_category_title_font_size() const { return category_title_font_size_; }
double PeriodTexConfig::get_margin_in() const { return margin_in_; }
double PeriodTexConfig::get_list_top_sep_pt() const { return list_top_sep_pt_; }
double PeriodTexConfig::get_list_item_sep_ex() const { return list_item_sep_ex_; }