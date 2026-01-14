// reports/period/formatters/latex/PeriodTexConfig.hpp
#ifndef PERIOD_TEX_CONFIG_HPP
#define PERIOD_TEX_CONFIG_HPP

#include "reports/period/common/PeriodBaseConfig.hpp"
#include "reports/shared/config/TexStyleConfig.hpp"
#include <string>
#include <nlohmann/json.hpp>

class PeriodTexConfig : public PeriodBaseConfig {
public:
    // [修改] 接收 JSON
    explicit PeriodTexConfig(const nlohmann::json& config);

    // 代理
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

#endif // PERIOD_TEX_CONFIG_HPP