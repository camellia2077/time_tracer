// queries/monthly/formatters/typ/MonthTypConfig.hpp
#ifndef MONTH_TYP_CONFIG_HPP
#define MONTH_TYP_CONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>

class MonthTypConfig {
public:
    explicit MonthTypConfig(const std::string& config_path);

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
    // --- [核心修改] ---
    const std::string& get_report_title() const; // 重命名
    const std::string& get_actual_days_label() const;
    const std::string& get_total_time_label() const;
    const std::string& get_invalid_format_message() const;
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
    // --- [核心修改] ---
    std::string report_title_; // 重命名
    std::string actual_days_label_;
    std::string total_time_label_;
    std::string invalid_format_message_;
    std::string no_records_;
};

#endif // MONTH_TYP_CONFIG_HPP