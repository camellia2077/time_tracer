// reports/monthly/formatters/latex/month_tex_config.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_CONFIG_HPP_
#define REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_CONFIG_HPP_

#include "reports/monthly/common/month_base_config.hpp"
#include "reports/shared/config/tex_style_config.hpp"
#include <string>
#include <toml++/toml.h> 

class MonthTexConfig : public MonthBaseConfig {
public:
    explicit MonthTexConfig(const toml::table& config);
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

#endif // REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_CONFIG_HPP_