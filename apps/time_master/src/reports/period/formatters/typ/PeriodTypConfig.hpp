// reports/period/formatters/typ/PeriodTypConfig.hpp
#ifndef PERIOD_TYP_CONFIG_HPP
#define PERIOD_TYP_CONFIG_HPP

#include "reports/period/formatters/base/PeriodBaseConfig.hpp"
#include <string>

class PeriodTypConfig : public PeriodBaseConfig {
public:
    explicit PeriodTypConfig(const std::string& config_path);

    // --- 特有的 Getters ---
    const std::string& get_base_font() const;
    const std::string& get_title_font() const;
    const std::string& get_category_title_font() const;
    int get_base_font_size() const;
    int get_report_title_font_size() const;
    int get_category_title_font_size() const;
    double get_line_spacing_em() const;
    double get_margin_top_cm() const;
    double get_margin_bottom_cm() const;
    double get_margin_left_cm() const;
    double get_margin_right_cm() const;

private:
    // --- 特有的成员变量 ---
    std::string base_font_;
    std::string title_font_;
    std::string category_title_font_;
    int base_font_size_;
    int report_title_font_size_;
    int category_title_font_size_;
    double line_spacing_em_;
    double margin_top_cm_;
    double margin_bottom_cm_;
    double margin_left_cm_;
    double margin_right_cm_;
};

#endif // PERIOD_TYP_CONFIG_HPP