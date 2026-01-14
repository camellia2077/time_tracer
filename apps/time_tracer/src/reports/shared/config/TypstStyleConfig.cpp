// reports/shared/config/TypstStyleConfig.cpp
#include "TypstStyleConfig.hpp"

TypstStyleConfig::TypstStyleConfig(const nlohmann::json& json) {
    base_font_ = json.at("base_font").get<std::string>();
    title_font_ = json.at("title_font").get<std::string>();
    category_title_font_ = json.at("category_title_font").get<std::string>();
    base_font_size_ = json.at("base_font_size").get<int>();
    report_title_font_size_ = json.at("report_title_font_size").get<int>();
    category_title_font_size_ = json.at("category_title_font_size").get<int>();
    line_spacing_em_ = json.at("line_spacing_em").get<double>();
}

const std::string& TypstStyleConfig::get_base_font() const { return base_font_; }
const std::string& TypstStyleConfig::get_title_font() const { return title_font_; }
const std::string& TypstStyleConfig::get_category_title_font() const { return category_title_font_; }
int TypstStyleConfig::get_base_font_size() const { return base_font_size_; }
int TypstStyleConfig::get_report_title_font_size() const { return report_title_font_size_; }
int TypstStyleConfig::get_category_title_font_size() const { return category_title_font_size_; }
double TypstStyleConfig::get_line_spacing_em() const { return line_spacing_em_; }