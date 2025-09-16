#include "MonthTypConfig.hpp"
#include <fstream>
#include <stdexcept>

MonthTypConfig::MonthTypConfig(const std::string& config_path) {
    load_config(config_path);
}

void MonthTypConfig::load_config(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open Typst monthly report config file: " + config_path);
    }
    nlohmann::json config_json;
    config_file >> config_json;

    body_font_ = config_json.at("BodyFont").get<std::string>();
    title_font_ = config_json.at("TitleFont").get<std::string>();
    title_font_size_ = config_json.at("TitleFontSize").get<int>();
    title_prefix_ = config_json.at("TitlePrefix").get<std::string>();
    actual_days_label_ = config_json.at("ActualDaysLabel").get<std::string>();
    total_time_label_ = config_json.at("TotalTimeLabel").get<std::string>();
    invalid_format_error_ = config_json.at("InvalidFormatError").get<std::string>();
    no_records_ = config_json.at("NoRecords").get<std::string>();
}

const std::string& MonthTypConfig::get_body_font() const { return body_font_; }
const std::string& MonthTypConfig::get_title_font() const { return title_font_; }
int MonthTypConfig::get_title_font_size() const { return title_font_size_; }
const std::string& MonthTypConfig::get_title_prefix() const { return title_prefix_; }
const std::string& MonthTypConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& MonthTypConfig::get_total_time_label() const { return total_time_label_; }
const std::string& MonthTypConfig::get_invalid_format_error() const { return invalid_format_error_; }
const std::string& MonthTypConfig::get_no_records() const { return no_records_; }