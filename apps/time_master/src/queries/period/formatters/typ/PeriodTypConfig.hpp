// queries/period/formatters/typ/PeriodTypConfig.hpp
#ifndef PERIOD_TYP_CONFIG_HPP
#define PERIOD_TYP_CONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>

class PeriodTypConfig {
public:
    explicit PeriodTypConfig(const std::string& config_path);

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
    const std::string& get_title_prefix() const;
    const std::string& get_title_days_unit() const;
    const std::string& get_title_date_separator() const;
    const std::string& get_total_time_label() const;
    const std::string& get_actual_days_label() const;
    const std::string& get_invalid_days_message() const;
    const std::string& get_no_records() const;

private:
    void load_config(const std::string& config_path);

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
    std::string title_prefix_;
    std::string title_days_unit_;
    std::string title_date_separator_;
    std::string total_time_label_;
    std::string actual_days_label_;
    std::string invalid_days_message_;
    std::string no_records_;
};

#endif // PERIOD_TYP_CONFIG_HPP