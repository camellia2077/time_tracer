// reports/shared/config/TexStyleConfig.cpp
#include "TexStyleConfig.hpp"

TexStyleConfig::TexStyleConfig(const nlohmann::json& json) {
    main_font_ = json.at("main_font").get<std::string>();
    cjk_main_font_ = json.at("cjk_main_font").get<std::string>();
    base_font_size_ = json.at("base_font_size").get<int>();
    report_title_font_size_ = json.at("report_title_font_size").get<int>();
    category_title_font_size_ = json.at("category_title_font_size").get<int>();
    margin_in_ = json.at("margin_in").get<double>();
    list_top_sep_pt_ = json.at("list_top_sep_pt").get<double>();
    list_item_sep_ex_ = json.at("list_item_sep_ex").get<double>();
}

const std::string& TexStyleConfig::get_main_font() const { return main_font_; }
const std::string& TexStyleConfig::get_cjk_main_font() const { return cjk_main_font_; }
int TexStyleConfig::get_base_font_size() const { return base_font_size_; }
int TexStyleConfig::get_report_title_font_size() const { return report_title_font_size_; }
int TexStyleConfig::get_category_title_font_size() const { return category_title_font_size_; }
double TexStyleConfig::get_margin_in() const { return margin_in_; }
double TexStyleConfig::get_list_top_sep_pt() const { return list_top_sep_pt_; }
double TexStyleConfig::get_list_item_sep_ex() const { return list_item_sep_ex_; }