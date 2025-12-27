// reports/daily/formatters/tex/DayTexConfig.cpp
#include "DayTexConfig.hpp"

// [修改] 构造函数调用基类构造函数，然后只加载自己的特有项
DayTexConfig::DayTexConfig(const std::string& config_path)
    : DayBaseConfig(config_path) // 调用基类构造函数，加载所有通用配置
{
    // 使用基类中已加载的 config_json_ 来加载本类特有的配置项
    report_title_ = config_json_.at("report_title").get<std::string>();
    main_font_ = config_json_.at("main_font").get<std::string>();
    cjk_main_font_ = config_json_.at("cjk_main_font").get<std::string>();
    base_font_size_ = config_json_.at("base_font_size").get<int>();
    report_title_font_size_ = config_json_.at("report_title_font_size").get<int>();
    category_title_font_size_ = config_json_.at("category_title_font_size").get<int>();
    margin_in_ = config_json_.at("margin_in").get<double>();
    list_top_sep_pt_ = config_json_.at("list_top_sep_pt").get<double>();
    list_item_sep_ex_ = config_json_.at("list_item_sep_ex").get<double>();
    keyword_colors_ = config_json_.at("keyword_colors").get<std::map<std::string, std::string>>();
}

// --- [修改] 只实现特有的 getters ---
const std::string& DayTexConfig::get_report_title() const { return report_title_; }
const std::string& DayTexConfig::get_main_font() const { return main_font_; }
const std::string& DayTexConfig::get_cjk_main_font() const { return cjk_main_font_; }
int DayTexConfig::get_base_font_size() const { return base_font_size_; }
int DayTexConfig::get_report_title_font_size() const { return report_title_font_size_; }
int DayTexConfig::get_category_title_font_size() const { return category_title_font_size_; }
double DayTexConfig::get_margin_in() const { return margin_in_; }
double DayTexConfig::get_list_top_sep_pt() const { return list_top_sep_pt_; }
double DayTexConfig::get_list_item_sep_ex() const { return list_item_sep_ex_; }
const std::map<std::string, std::string>& DayTexConfig::get_keyword_colors() const { return keyword_colors_; }