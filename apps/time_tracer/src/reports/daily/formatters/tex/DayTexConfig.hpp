// reports/daily/formatters/tex/DayTexConfig.hpp
#ifndef DAY_TEX_CONFIG_HPP
#define DAY_TEX_CONFIG_HPP

#include "reports/daily/formatters/base/DayBaseConfig.hpp" // [修改] 引入基类
#include <string>
#include <map>

// DayTexConfig 继承 DayBaseConfig
class DayTexConfig : public DayBaseConfig {
public:
    explicit DayTexConfig(const std::string& config_path);

    // --- [修改] 只保留特有的 getter ---
    const std::string& get_report_title() const; // Tex 特有的 report_title
    const std::string& get_main_font() const;
    const std::string& get_cjk_main_font() const;
    int get_base_font_size() const;
    int get_report_title_font_size() const;
    int get_category_title_font_size() const;
    double get_margin_in() const;
    double get_list_top_sep_pt() const;
    double get_list_item_sep_ex() const;
    const std::map<std::string, std::string>& get_keyword_colors() const;

private:
    // --- [修改] 只保留特有的成员变量 ---
    std::string report_title_; // 与基类的 title_prefix_ 不同
    std::string main_font_;
    std::string cjk_main_font_;
    int base_font_size_;
    int report_title_font_size_;
    int category_title_font_size_;
    double margin_in_;
    double list_top_sep_pt_;
    double list_item_sep_ex_;
    std::map<std::string, std::string> keyword_colors_;
};

#endif // DAY_TEX_CONFIG_HPP