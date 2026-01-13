// reports/daily/formatters/typst/DayTypConfig.hpp
#ifndef DAY_TYP_CONFIG_HPP
#define DAY_TYP_CONFIG_HPP

#include "reports/daily/common/DayBaseConfig.hpp"
#include "reports/shared/config/TypstStyleConfig.hpp" // [新增]
#include <string>
#include <map>
#include <filesystem>

class DayTypConfig : public DayBaseConfig {
public:
   explicit DayTypConfig(const std::filesystem::path& config_path);

    // --- Day 特有的 ---
    int get_statistic_font_size() const;
    int get_statistic_title_font_size() const;
    const std::map<std::string, std::string>& get_keyword_colors() const;
    
    // --- 代理给 style_ ---
    const std::string& get_title_font() const { return style_.get_title_font(); }
    const std::string& get_base_font() const { return style_.get_base_font(); }
    const std::string& get_category_title_font() const { return style_.get_category_title_font(); }
    int get_base_font_size() const { return style_.get_base_font_size(); }
    int get_report_title_font_size() const { return style_.get_report_title_font_size(); }
    int get_category_title_font_size() const { return style_.get_category_title_font_size(); }
    double get_line_spacing_em() const { return style_.get_line_spacing_em(); }

private:
    TypstStyleConfig style_; // [组合]
    int statistic_font_size_;
    int statistic_title_font_size_;
    std::map<std::string, std::string> keyword_colors_;
};

#endif // DAY_TYP_CONFIG_HPP