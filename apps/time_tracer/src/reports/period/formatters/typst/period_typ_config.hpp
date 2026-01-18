// reports/period/formatters/typst/period_typ_config.hpp
#ifndef REPORTS_PERIOD_FORMATTERS_TYPST_PERIOD_TYP_CONFIG_HPP_
#define REPORTS_PERIOD_FORMATTERS_TYPST_PERIOD_TYP_CONFIG_HPP_

#include "reports/period/common/period_base_config.hpp"
#include "reports/shared/config/typst_style_config.hpp"
#include <string>
#include <toml++/toml.h>

namespace reporting {

class PeriodTypConfig : public PeriodBaseConfig {
public:
    explicit PeriodTypConfig(const toml::table& config);

    // 代理
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

} // namespace reporting

#endif // REPORTS_PERIOD_FORMATTERS_TYPST_PERIOD_TYP_CONFIG_HPP_