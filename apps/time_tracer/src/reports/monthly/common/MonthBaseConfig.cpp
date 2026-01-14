// reports/monthly/common/MonthBaseConfig.cpp
#include "MonthBaseConfig.hpp"
#include <stdexcept>

// [修改] 接收 JSON，不再 IO
MonthBaseConfig::MonthBaseConfig(const nlohmann::json& config) 
    : config_json_(config)
{
    load_base_config();
}

void MonthBaseConfig::load_base_config() {
    report_title_ = config_json_.at("report_title").get<std::string>();
    actual_days_label_ = config_json_.at("actual_days_label").get<std::string>();
    total_time_label_ = config_json_.at("total_time_label").get<std::string>();
    invalid_format_message_ = config_json_.at("invalid_format_message").get<std::string>();
    no_records_message_ = config_json_.at("no_records_message").get<std::string>();
    project_breakdown_label_ = config_json_.value("project_breakdown_label", "Project Breakdown");
}

const std::string& MonthBaseConfig::get_report_title() const { return report_title_; }
const std::string& MonthBaseConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& MonthBaseConfig::get_total_time_label() const { return total_time_label_; }
const std::string& MonthBaseConfig::get_no_records_message() const { return no_records_message_; }
const std::string& MonthBaseConfig::get_invalid_format_message() const { return invalid_format_message_; }
const std::string& MonthBaseConfig::get_project_breakdown_label() const { return project_breakdown_label_; }
