// reports/monthly/formatters/typst/MonthTypConfig.hpp
#ifndef MONTH_TYP_CONFIG_HPP
#define MONTH_TYP_CONFIG_HPP

#include "reports/monthly/common/MonthBaseConfig.hpp"
#include "reports/shared/config/TypstStyleConfig.hpp" // [新增]
#include <string>
#include <filesystem>

class MonthTypConfig : public MonthBaseConfig {
public:
    explicit MonthTypConfig(const std::filesystem::path& config_path);

    // --- 代理 ---
    const std::string& get_base_font() const { return style_.get_base_font(); }
    const std::string& get_title_font() const { return style_.get_title_font(); }
    const std::string& get_category_title_font() const { return style_.get_category_title_font(); }
    int get_base_font_size() const { return style_.get_base_font_size(); }
    int get_report_title_font_size() const { return style_.get_report_title_font_size(); }
    int get_category_title_font_size() const { return style_.get_category_title_font_size(); }
    double get_line_spacing_em() const { return style_.get_line_spacing_em(); }

    // --- 特有的边距 ---
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

#endif // MONTH_TYP_CONFIG_HPP