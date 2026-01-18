// reports/monthly/common/month_base_config.cpp
#include "month_base_config.hpp"
#include <stdexcept>

MonthBaseConfig::MonthBaseConfig(const toml::table& config) 
    : config_table_(config)
{
    load_base_config();
}

void MonthBaseConfig::load_base_config() {
    // 使用 toml++ API 获取值
    // value_or 对于必填项如果不提供默认值会比较麻烦，这里假设配置文件是完整的
    // 或者提供默认空字符串以避免崩溃
    report_title_ = config_table_["report_title"].value_or<std::string>("");
    actual_days_label_ = config_table_["actual_days_label"].value_or<std::string>("");
    total_time_label_ = config_table_["total_time_label"].value_or<std::string>("");
    invalid_format_message_ = config_table_["invalid_format_message"].value_or<std::string>("");
    no_records_message_ = config_table_["no_records_message"].value_or<std::string>("");
    project_breakdown_label_ = config_table_["project_breakdown_label"].value_or<std::string>("Project Breakdown");
}

const std::string& MonthBaseConfig::get_report_title() const { return report_title_; }
const std::string& MonthBaseConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& MonthBaseConfig::get_total_time_label() const { return total_time_label_; }
const std::string& MonthBaseConfig::get_no_records_message() const { return no_records_message_; }
const std::string& MonthBaseConfig::get_invalid_format_message() const { return invalid_format_message_; }
const std::string& MonthBaseConfig::get_project_breakdown_label() const { return project_breakdown_label_; }