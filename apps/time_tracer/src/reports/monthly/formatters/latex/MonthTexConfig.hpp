// reports/monthly/formatters/latex/MonthTexConfig.hpp
#ifndef MONTH_TEX_CONFIG_HPP
#define MONTH_TEX_CONFIG_HPP

#include "reports/monthly/common/MonthBaseConfig.hpp"
#include "reports/shared/config/TexStyleConfig.hpp"
#include <string>
#include <toml++/toml.h> // [修改]

class MonthTexConfig : public MonthBaseConfig {
public:
    // [修改] 接收 toml::table
    explicit MonthTexConfig(const toml::table& config);

    // 代理 (TexStyleConfig 已经在 shared 中更新为 TOML 支持)
    const std::string& get_main_font() const { return style_.get_main_font(); }
    const std::string& get_cjk_main_font() const { return style_.get_cjk_main_font(); }
    int get_base_font_size() const { return style_.get_base_font_size(); }
    int get_report_title_font_size() const { return style_.get_report_title_font_size(); }
    int get_category_title_font_size() const { return style_.get_category_title_font_size(); }
    double get_margin_in() const { return style_.get_margin_in(); }
    double get_list_top_sep_pt() const { return style_.get_list_top_sep_pt(); }
    double get_list_item_sep_ex() const { return style_.get_list_item_sep_ex(); }

private:
    TexStyleConfig style_;
};

#endif // MONTH_TEX_CONFIG_HPP