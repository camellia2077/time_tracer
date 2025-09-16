#ifndef MONTH_TYP_CONFIG_HPP
#define MONTH_TYP_CONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>

class MonthTypConfig {
public:
    explicit MonthTypConfig(const std::string& config_path);

    const std::string& get_body_font() const;
    const std::string& get_title_font() const;
    int get_title_font_size() const;
    const std::string& get_title_prefix() const;
    const std::string& get_actual_days_label() const;
    const std::string& get_total_time_label() const;
    const std::string& get_invalid_format_error() const;
    const std::string& get_no_records() const;

private:
    void load_config(const std::string& config_path);

    std::string body_font_;
    std::string title_font_;
    int title_font_size_;
    std::string title_prefix_;
    std::string actual_days_label_;
    std::string total_time_label_;
    std::string invalid_format_error_;
    std::string no_records_;
};

#endif // MONTH_TYP_CONFIG_HPP