// queries/monthly/formatters/tex/MonthTexConfig.cpp
#include "MonthTexConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp" // [新增]
#include <stdexcept>

MonthTexConfig::MonthTexConfig(const std::string& config_path) {
    load_config(config_path);
}

void MonthTexConfig::load_config(const std::string& config_path) {
    // [修改] 使用新的辅助函数
    nlohmann::json config_json = load_json_config(config_path, "Could not open MonthTexConfig file: ");

    report_title_ = config_json.at("ReportTitle").get<std::string>();
    actual_days_label_ = config_json.at("ActualDaysLabel").get<std::string>();
    total_time_label_ = config_json.at("TotalTimeLabel").get<std::string>();
    no_records_message_ = config_json.at("NoRecordsMessage").get<std::string>();
    invalid_format_message_ = config_json.at("InvalidFormatMessage").get<std::string>();
    compact_list_options_ = config_json.at("CompactListOptions").get<std::string>();
    main_font_ = config_json.at("MainFont").get<std::string>();
    cjk_main_font_ = config_json.at("CJKMainFont").get<std::string>();
}

// Getters
const std::string& MonthTexConfig::get_report_title() const { return report_title_; }
const std::string& MonthTexConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& MonthTexConfig::get_total_time_label() const { return total_time_label_; }
const std::string& MonthTexConfig::get_no_records_message() const { return no_records_message_; }
const std::string& MonthTexConfig::get_invalid_format_message() const { return invalid_format_message_; }
const std::string& MonthTexConfig::get_compact_list_options() const { return compact_list_options_; }
const std::string& MonthTexConfig::get_main_font() const { return main_font_; }
const std::string& MonthTexConfig::get_cjk_main_font() const { return cjk_main_font_; }