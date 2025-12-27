// reports/monthly/formatters/tex/MonthTexConfig.hpp
#ifndef MONTH_TEX_CONFIG_HPP
#define MONTH_TEX_CONFIG_HPP

#include "reports/monthly/formatters/base/MonthBaseConfig.hpp"
#include <string>

class MonthTexConfig : public MonthBaseConfig {
public:
    explicit MonthTexConfig(const std::string& config_path);

    // --- 特有的 Getters ---
    const std::string& get_main_font() const;
    const std::string& get_cjk_main_font() const;
    int get_base_font_size() const;
    int get_report_title_font_size() const;
    int get_category_title_font_size() const;
    double get_margin_in() const;
    double get_list_top_sep_pt() const;
    double get_list_item_sep_ex() const;

private:
    // --- 特有的成员变量 ---
    std::string main_font_;
    std::string cjk_main_font_;
    int base_font_size_;
    int report_title_font_size_;
    int category_title_font_size_;
    double margin_in_;
    double list_top_sep_pt_;
    double list_item_sep_ex_;
};

#endif // MONTH_TEX_CONFIG_HPP