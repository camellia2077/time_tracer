// reports/period/common/PeriodBaseConfig.cpp
#include "PeriodBaseConfig.hpp"
#include <stdexcept>

PeriodBaseConfig::PeriodBaseConfig(const std::filesystem::path& config_path) {
    config_json_ = load_json_config(config_path, "Could not open Period report config file: ");
    load_base_config();
}

void PeriodBaseConfig::load_base_config() {
    // 加载逻辑保持不变
    report_title_prefix_ = config_json_.value("report_title_prefix", "Report for last");
    report_title_days_ = config_json_.value("report_title_days", "days");
    report_title_date_separator_ = config_json_.value("report_title_date_separator", "-");

    actual_days_label_ = config_json_.at("actual_days_label").get<std::string>();
    total_time_label_ = config_json_.at("total_time_label").get<std::string>();
    no_records_message_ = config_json_.at("no_records_message").get<std::string>();
    invalid_days_message_ = config_json_.at("invalid_days_message").get<std::string>();

    project_breakdown_label_ = config_json_.value("project_breakdown_label", "Project Breakdown");
}

// Getters 实现保持不变
const std::string& PeriodBaseConfig::get_report_title_prefix() const { return report_title_prefix_; }
const std::string& PeriodBaseConfig::get_report_title_days() const { return report_title_days_; }
const std::string& PeriodBaseConfig::get_report_title_date_separator() const { return report_title_date_separator_; }

const std::string& PeriodBaseConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& PeriodBaseConfig::get_total_time_label() const { return total_time_label_; }
const std::string& PeriodBaseConfig::get_no_records_message() const { return no_records_message_; }
const std::string& PeriodBaseConfig::get_invalid_days_message() const { return invalid_days_message_; }
const std::string& PeriodBaseConfig::get_project_breakdown_label() const { return project_breakdown_label_; }