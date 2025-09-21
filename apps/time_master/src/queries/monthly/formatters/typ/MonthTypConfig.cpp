// queries/monthly/formatters/typ/MonthTypConfig.cpp
#include "MonthTypConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp"
#include <stdexcept>

MonthTypConfig::MonthTypConfig(const std::string& config_path) {
    load_config(config_path);
}

void MonthTypConfig::load_config(const std::string& config_path) {
    nlohmann::json config_json = load_json_config(config_path, "Could not open Typst monthly report config file: ");

    base_font_ = config_json.at("base_font").get<std::string>();
    title_font_ = config_json.at("title_font").get<std::string>();
    category_title_font_ = config_json.at("category_title_font").get<std::string>();
    base_font_size_ = config_json.at("base_font_size").get<int>();
    report_title_font_size_ = config_json.at("report_title_font_size").get<int>();
    category_title_font_size_ = config_json.at("category_title_font_size").get<int>();
    line_spacing_em_ = config_json.at("line_spacing_em").get<double>();
    margin_top_cm_ = config_json.at("margin_top_cm").get<double>();
    margin_bottom_cm_ = config_json.at("margin_bottom_cm").get<double>();
    margin_left_cm_ = config_json.at("margin_left_cm").get<double>();
    margin_right_cm_ = config_json.at("margin_right_cm").get<double>();
    // --- [核心修改] ---
    report_title_ = config_json.at("report_title").get<std::string>(); // 修正键名
    actual_days_label_ = config_json.at("actual_days_label").get<std::string>();
    total_time_label_ = config_json.at("total_time_label").get<std::string>();
    invalid_format_message_ = config_json.at("invalid_format_message").get<std::string>();
    no_records_ = config_json.at("no_records").get<std::string>();
}

// Getters
const std::string& MonthTypConfig::get_base_font() const { return base_font_; }
const std::string& MonthTypConfig::get_title_font() const { return title_font_; }
const std::string& MonthTypConfig::get_category_title_font() const { return category_title_font_; }
int MonthTypConfig::get_base_font_size() const { return base_font_size_; }
int MonthTypConfig::get_report_title_font_size() const { return report_title_font_size_; }
int MonthTypConfig::get_category_title_font_size() const { return category_title_font_size_; }
double MonthTypConfig::get_line_spacing_em() const { return line_spacing_em_; }
double MonthTypConfig::get_margin_top_cm() const { return margin_top_cm_; }
double MonthTypConfig::get_margin_bottom_cm() const { return margin_bottom_cm_; }
double MonthTypConfig::get_margin_left_cm() const { return margin_left_cm_; }
double MonthTypConfig::get_margin_right_cm() const { return margin_right_cm_; }
// --- [核心修改] ---
const std::string& MonthTypConfig::get_report_title() const { return report_title_; } // 修正 Getter
const std::string& MonthTypConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& MonthTypConfig::get_total_time_label() const { return total_time_label_; }
const std::string& MonthTypConfig::get_invalid_format_message() const { return invalid_format_message_; }
const std::string& MonthTypConfig::get_no_records() const { return no_records_; }