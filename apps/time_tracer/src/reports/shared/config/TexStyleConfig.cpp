// reports/shared/config/TexStyleConfig.cpp
#include "TexStyleConfig.hpp"

// [修改] 使用 toml::table 解析
TexStyleConfig::TexStyleConfig(const toml::table& tbl) {
    // [修复] 移除 value_or 后面的 <std::string>，让编译器自动推导类型
    // 这样就可以传递 main_font_ (左值) 作为默认值了
    main_font_ = tbl["main_font"].value_or(""); 
    cjk_main_font_ = tbl["cjk_main_font"].value_or(main_font_); 
    
    base_font_size_ = tbl["base_font_size"].value_or(10);
    report_title_font_size_ = tbl["report_title_font_size"].value_or(14);
    category_title_font_size_ = tbl["category_title_font_size"].value_or(12);
    margin_in_ = tbl["margin_in"].value_or(1.0);
    list_top_sep_pt_ = tbl["list_top_sep_pt"].value_or(0.0);
    list_item_sep_ex_ = tbl["list_item_sep_ex"].value_or(0.0);
}

const std::string& TexStyleConfig::get_main_font() const { return main_font_; }
const std::string& TexStyleConfig::get_cjk_main_font() const { return cjk_main_font_; }
int TexStyleConfig::get_base_font_size() const { return base_font_size_; }
int TexStyleConfig::get_report_title_font_size() const { return report_title_font_size_; }
int TexStyleConfig::get_category_title_font_size() const { return category_title_font_size_; }
double TexStyleConfig::get_margin_in() const { return margin_in_; }
double TexStyleConfig::get_list_top_sep_pt() const { return list_top_sep_pt_; }
double TexStyleConfig::get_list_item_sep_ex() const { return list_item_sep_ex_; }