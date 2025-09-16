// queries/monthly/formatters/md/MonthMdConfig.cpp
#include "MonthMdConfig.hpp"
#include "queries/shared/utils/ConfigUtils.hpp" // [新增]
#include <stdexcept>

MonthMdConfig::MonthMdConfig(const std::string& config_path) {
    load_config(config_path);
}

void MonthMdConfig::load_config(const std::string& config_path) {
    // [修改] 使用新的辅助函数
    nlohmann::json config_json = load_json_config(config_path, "Could not open MonthMdConfig file: ");

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