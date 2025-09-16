#include "PeriodMdConfig.hpp"
#include <fstream>
#include <stdexcept>

PeriodMdConfig::PeriodMdConfig(const std::string& config_path) {
    load_config(config_path);
}

void PeriodMdConfig::load_config(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open PeriodMdConfig file: " + config_path);
    }
    nlohmann::json config_json;
    config_file >> config_json;

    report_title_prefix_ = config_json.at("ReportTitlePrefix").get<std::string>();
    report_title_days_ = config_json.at("ReportTitleDays").get<std::string>();
    report_title_date_separator_ = config_json.at("ReportTitleDateSeparator").get<std::string>();
    total_time_label_ = config_json.at("TotalTimeLabel").get<std::string>();
    actual_days_label_ = config_json.at("ActualDaysLabel").get<std::string>();
    no_records_message_ = config_json.at("NoRecordsMessage").get<std::string>();
    invalid_days_message_ = config_json.at("InvalidDaysMessage").get<std::string>();
}

const std::string& PeriodMdConfig::get_report_title_prefix() const { return report_title_prefix_; }
const std::string& PeriodMdConfig::get_report_title_days() const { return report_title_days_; }
const std::string& PeriodMdConfig::get_report_title_date_separator() const { return report_title_date_separator_; }
const std::string& PeriodMdConfig::get_total_time_label() const { return total_time_label_; }
const std::string& PeriodMdConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& PeriodMdConfig::get_no_records_message() const { return no_records_message_; }
const std::string& PeriodMdConfig::get_invalid_days_message() const { return invalid_days_message_; }