// reports/period/formatters/base/PeriodBaseConfig.hpp
#ifndef PERIOD_BASE_CONFIG_HPP
#define PERIOD_BASE_CONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>
#include "reports/shared/utils/config/ConfigUtils.hpp"

class PeriodBaseConfig {
public:
    explicit PeriodBaseConfig(const std::string& config_path);
    virtual ~PeriodBaseConfig() = default;

    // --- 通用配置项的 Getters ---
    const std::string& get_report_title_prefix() const;
    const std::string& get_report_title_days() const;
    const std::string& get_report_title_date_separator() const;
    const std::string& get_total_time_label() const;
    const std::string& get_actual_days_label() const;
    const std::string& get_no_records_message() const;
    const std::string& get_invalid_days_message() const;

protected:
    nlohmann::json config_json_;

private:
    void load_base_config();

    // --- 共享的成员变量 ---
    std::string report_title_prefix_;
    std::string report_title_days_;
    std::string report_title_date_separator_;
    std::string total_time_label_;
    std::string actual_days_label_;
    std::string no_records_message_;
    std::string invalid_days_message_;
};

#endif // PERIOD_BASE_CONFIG_HPP