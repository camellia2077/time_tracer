// reports/period/common/PeriodBaseConfig.cpp
#include "PeriodBaseConfig.hpp"
#include <stdexcept>

// [修改] 接收 toml::table
PeriodBaseConfig::PeriodBaseConfig(const toml::table& config) 
    : config_table_(config)
{
    load_base_config();
}

void PeriodBaseConfig::load_base_config() {
    report_title_prefix_ = config_table_["report_title_prefix"].value_or<std::string>("Report for last");
    report_title_days_ = config_table_["report_title_days"].value_or<std::string>("days");
    report_title_date_separator_ = config_table_["report_title_date_separator"].value_or<std::string>("-");

    // 注意：value_or("") 用于避免因缺失键导致崩溃，实际逻辑中可以使用 get_required 辅助函数
    actual_days_label_ = config_table_["actual_days_label"].value_or<std::string>("");
    total_time_label_ = config_table_["total_time_label"].value_or<std::string>("");
    no_records_message_ = config_table_["no_records_message"].value_or<std::string>("");
    invalid_days_message_ = config_table_["invalid_days_message"].value_or<std::string>("");

    project_breakdown_label_ = config_table_["project_breakdown_label"].value_or<std::string>("Project Breakdown");
}

const std::string& PeriodBaseConfig::get_report_title_prefix() const { return report_title_prefix_; }
const std::string& PeriodBaseConfig::get_report_title_days() const { return report_title_days_; }
const std::string& PeriodBaseConfig::get_report_title_date_separator() const { return report_title_date_separator_; }
const std::string& PeriodBaseConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& PeriodBaseConfig::get_total_time_label() const { return total_time_label_; }
const std::string& PeriodBaseConfig::get_no_records_message() const { return no_records_message_; }
const std::string& PeriodBaseConfig::get_invalid_days_message() const { return invalid_days_message_; }
const std::string& PeriodBaseConfig::get_project_breakdown_label() const { return project_breakdown_label_; }