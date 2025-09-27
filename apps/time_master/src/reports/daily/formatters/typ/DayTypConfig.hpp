// reports/daily/formatters/typ/DayTypConfig.hpp
#ifndef DAY_TYP_CONFIG_HPP
#define DAY_TYP_CONFIG_HPP

#include "reports/daily/formatters/base/DayBaseConfig.hpp" // [修改] 引入基类
#include <string>
#include <map>

// DayTypConfig 继承 DayBaseConfig
class DayTypConfig : public DayBaseConfig {
public:
    explicit DayTypConfig(const std::string& config_path);

    // --- [修改] 只保留特有的 getter ---
    const std::string& get_title_font() const;
    const std::string& get_base_font() const;
    const std::string& get_category_title_font() const;
    int get_base_font_size() const;
    int get_report_title_font_size() const;
    int get_category_title_font_size() const;
    int get_statistic_font_size() const;
    int get_statistic_title_font_size() const;
    double get_line_spacing_em() const;
    const std::map<std::string, std::string>& get_keyword_colors() const;
    
private:
    // --- [修改] 只保留特有的成员变量 ---
    std::string title_font_;
    std::string base_font_;
    std::string category_title_font_;
    int base_font_size_;
    int report_title_font_size_;
    int category_title_font_size_;
    int statistic_font_size_;
    int statistic_title_font_size_;
    double line_spacing_em_;
    std::map<std::string, std::string> keyword_colors_;
};

#endif // DAY_TYP_CONFIG_HPP