// reports/daily/formatters/latex/DayTexConfig.hpp
#ifndef DAY_TEX_CONFIG_HPP
#define DAY_TEX_CONFIG_HPP

#include "reports/daily/common/DayBaseConfig.hpp"
#include "reports/shared/config/TexStyleConfig.hpp"
#include <string>
#include <map>
#include <nlohmann/json.hpp>

class DayTexConfig : public DayBaseConfig {
public:
    // [修改] 接收 JSON
    explicit DayTexConfig(const nlohmann::json& config);

    const std::string& get_report_title() const;
    const std::map<std::string, std::string>& get_keyword_colors() const;

    // 代理给 style_ 的方法
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
    std::string report_title_;
    std::map<std::string, std::string> keyword_colors_;
};

#endif // DAY_TEX_CONFIG_HPP