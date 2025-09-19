// queries/period/formatters/typ/PeriodTypConfig.cpp
#include "PeriodTypConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp" // [新增]
#include <stdexcept>

PeriodTypConfig::PeriodTypConfig(const std::string& config_path) {
    load_config(config_path);
}

void PeriodTypConfig::load_config(const std::string& config_path) {
    // [修改] 使用新的辅助函数
    nlohmann::json config_json = load_json_config(config_path, "Could not open Typst period report config file: ");

    title_font_ = config_json.at("TitleFont").get<std::string>();
    title_font_size_ = config_json.at("TitleFontSize").get<int>();
    content_font_ = config_json.at("ContentFont").get<std::string>();
    title_prefix_ = config_json.at("TitlePrefix").get<std::string>();
    total_time_label_ = config_json.at("TotalTimeLabel").get<std::string>();
    actual_days_label_ = config_json.at("ActualDaysLabel").get<std::string>();
    positive_days_error_ = config_json.at("PositiveDaysError").get<std::string>();
    no_records_ = config_json.at("NoRecords").get<std::string>();
}

const std::string& PeriodTypConfig::get_title_font() const { return title_font_; }
int PeriodTypConfig::get_title_font_size() const { return title_font_size_; }
const std::string& PeriodTypConfig::get_content_font() const { return content_font_; }
const std::string& PeriodTypConfig::get_title_prefix() const { return title_prefix_; }
const std::string& PeriodTypConfig::get_total_time_label() const { return total_time_label_; }
const std::string& PeriodTypConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& PeriodTypConfig::get_positive_days_error() const { return positive_days_error_; }
const std::string& PeriodTypConfig::get_no_records() const { return no_records_; }