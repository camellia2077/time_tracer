// queries/period/formatters/tex/PeriodTexConfig.hpp
#ifndef PERIOD_TEX_CONFIG_HPP
#define PERIOD_TEX_CONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>

class PeriodTexConfig {
public:
    explicit PeriodTexConfig(const std::string& config_path);

    const std::string& get_report_title_prefix() const;
    const std::string& get_report_title_days() const;
    const std::string& get_report_title_date_separator() const;
    const std::string& get_total_time_label() const;
    const std::string& get_actual_days_label() const;
    const std::string& get_no_records_message() const;
    const std::string& get_invalid_days_message() const;
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

    std::string report_title_prefix_;
    std::string report_title_days_;
    std::string report_title_date_separator_;
    std::string total_time_label_;
    std::string actual_days_label_;
    std::string no_records_message_;
    std::string invalid_days_message_;
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