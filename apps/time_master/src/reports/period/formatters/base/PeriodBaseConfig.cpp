#include "PeriodBaseConfig.hpp"
#include <stdexcept>

PeriodBaseConfig::PeriodBaseConfig(const std::string& config_path) {
    config_json_ = load_json_config(config_path, "Could not open Period report config file: ");
    load_base_config();
}

void PeriodBaseConfig::load_base_config() {
    report_title_prefix_ = config_json_.at("report_title_prefix").get<std::string>();
    report_title_days_ = config_json_.at("report_title_days").get<std::string>();
    report_title_date_separator_ = config_json_.at("report_title_date_separator").get<std::string>();
    total_time_label_ = config_json_.at("total_time_label").get<std::string>();
    actual_days_label_ = config_json_.at("actual_days_label").get<std::string>();
    invalid_days_message_ = config_json_.at("invalid_days_message").get<std::string>();

    // [核心修改] 移除兼容代码，直接加载 "no_records_message"
    no_records_message_ = config_json_.at("no_records_message").get<std::string>();
}

// --- Getters 实现保持不变 ---
const std::string& PeriodBaseConfig::get_report_title_prefix() const { return report_title_prefix_; }
const std::string& PeriodBaseConfig::get_report_title_days() const { return report_title_days_; }
const std::string& PeriodBaseConfig::get_report_title_date_separator() const { return report_title_date_separator_; }
const std::string& PeriodBaseConfig::get_total_time_label() const { return total_time_label_; }
const std::string& PeriodBaseConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& PeriodBaseConfig::get_no_records_message() const { return no_records_message_; }
const std::string& PeriodBaseConfig::get_invalid_days_message() const { return invalid_days_message_; }