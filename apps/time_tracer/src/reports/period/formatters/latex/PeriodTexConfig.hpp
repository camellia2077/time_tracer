// reports/period/formatters/latex/PeriodTexConfig.hpp
#ifndef PERIOD_TEX_CONFIG_HPP
#define PERIOD_TEX_CONFIG_HPP

#include "reports/period/common/PeriodBaseConfig.hpp"
#include <string>
#include <filesystem> 

class PeriodTexConfig : public PeriodBaseConfig {
public:
    explicit PeriodTexConfig(const std::filesystem::path& config_path);

    // Getters 保持不变
    const std::string& get_main_font() const;
    const std::string& get_cjk_main_font() const;
    int get_base_font_size() const;
    int get_report_title_font_size() const;
    int get_category_title_font_size() const;
    double get_margin_in() const;
    double get_list_top_sep_pt() const;
    double get_list_item_sep_ex() const;

private:
    std::string main_font_;
    std::string cjk_main_font_;
    int base_font_size_;
    int report_title_font_size_;
    int category_title_font_size_;
    double margin_in_;
    double list_top_sep_pt_;
    double list_item_sep_ex_;
};

#endif // PERIOD_TEX_CONFIG_HPP