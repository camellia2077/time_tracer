// reports/monthly/formatters/tex/MonthTexConfig.hpp
#ifndef MONTH_TEX_CONFIG_HPP
#define MONTH_TEX_CONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>

class MonthTexConfig {
public:
    explicit MonthTexConfig(const std::string& config_path);

    const std::string& get_report_title() const;
    const std::string& get_actual_days_label() const;
    const std::string& get_total_time_label() const;
    const std::string& get_no_records_message() const;
    const std::string& get_invalid_format_message() const;
    const std::string& get_main_font() const;
    const std::string& get_cjk_main_font() const;
    int get_base_font_size() const;
    int get_report_title_font_size() const;
    int get_category_title_font_size() const;
    double get_margin_in() const;
    double get_list_top_sep_pt() const;
    double get_list_item_sep_ex() const;

private:
    void load_config(const std::string& config_path);

    std::string report_title_;
    std::string actual_days_label_;
    std::string total_time_label_;
    std::string no_records_message_;
    std::string invalid_format_message_;
    std::string main_font_;
    std::string cjk_main_font_;
    int base_font_size_;
    int report_title_font_size_;
    int category_title_font_size_;
    double margin_in_;
    double list_top_sep_pt_;
    double list_item_sep_ex_;
};

#endif // MONTH_TEX_CONFIG_HPP