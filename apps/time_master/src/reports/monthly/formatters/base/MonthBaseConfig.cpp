// reports/monthly/formatters/base/MonthBaseConfig.cpp
#include "MonthBaseConfig.hpp"
#include <stdexcept>

MonthBaseConfig::MonthBaseConfig(const std::string& config_path) {
    config_json_ = load_json_config(config_path, "Could not open Monthly report config file: ");
    load_base_config();
}

void MonthBaseConfig::load_base_config() {
    report_title_ = config_json_.at("report_title").get<std::string>();
    actual_days_label_ = config_json_.at("actual_days_label").get<std::string>();
    total_time_label_ = config_json_.at("total_time_label").get<std::string>();
    invalid_format_message_ = config_json_.at("invalid_format_message").get<std::string>();

    // [核心修改] 移除兼容代码，直接加载 "no_records_message"
    no_records_message_ = config_json_.at("no_records_message").get<std::string>();
}

// --- Getters 实现保持不变 ---
const std::string& MonthBaseConfig::get_report_title() const { return report_title_; }
const std::string& MonthBaseConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& MonthBaseConfig::get_total_time_label() const { return total_time_label_; }
const std::string& MonthBaseConfig::get_no_records_message() const { return no_records_message_; }
const std::string& MonthBaseConfig::get_invalid_format_message() const { return invalid_format_message_; }