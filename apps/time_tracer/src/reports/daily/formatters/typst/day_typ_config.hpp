// reports/daily/formatters/typst/day_typ_config.hpp
#ifndef REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_CONFIG_HPP_
#define REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_CONFIG_HPP_

#include "reports/daily/common/day_base_config.hpp"
#include "reports/shared/config/typst_style_config.hpp"
#include <string>
#include <map>
#include <toml++/toml.h>

class DayTypConfig : public DayBaseConfig {
public:
   explicit DayTypConfig(const toml::table& config);

    int get_statistic_font_size() const;
    int get_statistic_title_font_size() const;
    const std::map<std::string, std::string>& get_keyword_colors() const;
    
    // 代理
    const std::string& get_title_font() const { return style_.get_title_font(); }
    const std::string& get_base_font() const { return style_.get_base_font(); }
    const std::string& get_category_title_font() const { return style_.get_category_title_font(); }
    int get_base_font_size() const { return style_.get_base_font_size(); }
    int get_report_title_font_size() const { return style_.get_report_title_font_size(); }
    int get_category_title_font_size() const { return style_.get_category_title_font_size(); }
    double get_line_spacing_em() const { return style_.get_line_spacing_em(); }

private:
    TypstStyleConfig style_;
    int statistic_font_size_;
    int statistic_title_font_size_;
    std::map<std::string, std::string> keyword_colors_;
};

#endif // REPORTS_DAILY_FORMATTERS_TYPST_DAY_TYP_CONFIG_HPP_