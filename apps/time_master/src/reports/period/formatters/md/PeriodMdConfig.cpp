// reports/period/formatters/md/PeriodMdConfig.cpp
#include "PeriodMdConfig.hpp"
#include "reports/shared/utils/config/ConfigUtils.hpp" // [新增]
#include <stdexcept>

PeriodMdConfig::PeriodMdConfig(const std::string& config_path) {
    load_config(config_path);
}

void PeriodMdConfig::load_config(const std::string& config_path) {
    // [修改] 使用新的辅助函数
    nlohmann::json config_json = load_json_config(config_path, "Could not open PeriodMdConfig file: ");

    report_title_prefix_ = config_json.at("report_title_prefix").get<std::string>();
    report_title_days_ = config_json.at("report_title_days").get<std::string>();
    report_title_date_separator_ = config_json.at("report_title_date_separator").get<std::string>();
    total_time_label_ = config_json.at("total_time_label").get<std::string>();
    actual_days_label_ = config_json.at("actual_days_label").get<std::string>();
    no_records_message_ = config_json.at("no_records_message").get<std::string>();
    invalid_days_message_ = config_json.at("invalid_days_message").get<std::string>();
}

const std::string& PeriodMdConfig::get_report_title_prefix() const { return report_title_prefix_; }
const std::string& PeriodMdConfig::get_report_title_days() const { return report_title_days_; }
const std::string& PeriodMdConfig::get_report_title_date_separator() const { return report_title_date_separator_; }
const std::string& PeriodMdConfig::get_total_time_label() const { return total_time_label_; }
const std::string& PeriodMdConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& PeriodMdConfig::get_no_records_message() const { return no_records_message_; }
const std::string& PeriodMdConfig::get_invalid_days_message() const { return invalid_days_message_; }