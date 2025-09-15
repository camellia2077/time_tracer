#ifndef MONTH_MD_CONFIG_HPP
#define MONTH_MD_CONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>

class MonthMdConfig {
public:
    explicit MonthMdConfig(const std::string& config_path);

    const std::string& get_report_title() const;
    const std::string& get_actual_days_label() const;
    const std::string& get_total_time_label() const;
    const std::string& get_no_records_message() const;
    const std::string& get_invalid_format_message() const;

private:
    void load_config(const std::string& config_path);

    std::string report_title_;
    std::string actual_days_label_;
    std::string total_time_label_;
    std::string no_records_message_;
    std::string invalid_format_message_;
};

#endif // MONTH_MD_CONFIG_HPP