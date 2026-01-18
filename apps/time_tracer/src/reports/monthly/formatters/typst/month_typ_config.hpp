// reports/monthly/formatters/typst/month_typ_config.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_CONFIG_HPP_
#define REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_CONFIG_HPP_

#include "reports/monthly/common/month_base_config.hpp"
#include "reports/shared/config/typst_style_config.hpp"
#include <string>
#include <toml++/toml.h>

class MonthTypConfig : public MonthBaseConfig {
public:
    explicit MonthTypConfig(const toml::table& config);

    const std::string& get_base_font() const { return style_.get_base_font(); }
    const std::string& get_title_font() const { return style_.get_title_font(); }
    const std::string& get_category_title_font() const { return style_.get_category_title_font(); }
    int get_base_font_size() const { return style_.get_base_font_size(); }
    int get_report_title_font_size() const { return style_.get_report_title_font_size(); }
    int get_category_title_font_size() const { return style_.get_category_title_font_size(); }
    double get_line_spacing_em() const { return style_.get_line_spacing_em(); }

    double get_margin_top_cm() const;
    double get_margin_bottom_cm() const;
    double get_margin_left_cm() const;
    double get_margin_right_cm() const;

private:
    TypstStyleConfig style_;
    double margin_top_cm_;
    double margin_bottom_cm_;
    double margin_left_cm_;
    double margin_right_cm_;
};

#endif // REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_CONFIG_HPP_