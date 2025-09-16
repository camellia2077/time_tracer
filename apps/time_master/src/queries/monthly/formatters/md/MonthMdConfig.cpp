#include "MonthMdConfig.hpp"
#include <fstream>
#include <stdexcept>

MonthMdConfig::MonthMdConfig(const std::string& config_path) {
    load_config(config_path);
}

void MonthMdConfig::load_config(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open MonthMdConfig file: " + config_path);
    }
    nlohmann::json config_json;
    config_file >> config_json;

    report_title_ = config_json.at("ReportTitle").get<std::string>();
    actual_days_label_ = config_json.at("ActualDaysLabel").get<std::string>();
    total_time_label_ = config_json.at("TotalTimeLabel").get<std::string>();
    no_records_message_ = config_json.at("NoRecordsMessage").get<std::string>();
    invalid_format_message_ = config_json.at("InvalidFormatMessage").get<std::string>();
}

const std::string& MonthMdConfig::get_report_title() const { return report_title_; }
const std::string& MonthMdConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& MonthMdConfig::get_total_time_label() const { return total_time_label_; }
const std::string& MonthMdConfig::get_no_records_message() const { return no_records_message_; }
const std::string& MonthMdConfig::get_invalid_format_message() const { return invalid_format_message_; }