// queries/period/formatters/typ/PeriodTypConfig.cpp
#include "PeriodTypConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp"
#include <stdexcept>

PeriodTypConfig::PeriodTypConfig(const std::string& config_path) {
    load_config(config_path);
}

void PeriodTypConfig::load_config(const std::string& config_path) {
    nlohmann::json config_json = load_json_config(config_path, "Could not open Typst period report config file: ");

    base_font_ = config_json.at("BaseFont").get<std::string>();
    title_font_ = config_json.at("TitleFont").get<std::string>();
    base_font_size_ = config_json.at("BaseFontSize").get<int>();
    report_title_font_size_ = config_json.at("ReportTitleFontSize").get<int>();
    category_title_font_size_ = config_json.at("CategoryTitleFontSize").get<int>();
    line_spacing_em_ = config_json.at("LineSpacing_em").get<double>();
    margin_top_cm_ = config_json.at("Margin_top_cm").get<double>();
    margin_bottom_cm_ = config_json.at("Margin_bottom_cm").get<double>();
    margin_left_cm_ = config_json.at("Margin_left_cm").get<double>();
    margin_right_cm_ = config_json.at("Margin_right_cm").get<double>();
    title_prefix_ = config_json.at("TitlePrefix").get<std::string>();
    title_days_unit_ = config_json.at("TitleDaysUnit").get<std::string>();
    title_date_separator_ = config_json.at("TitleDateSeparator").get<std::string>();
    total_time_label_ = config_json.at("TotalTimeLabel").get<std::string>();
    actual_days_label_ = config_json.at("ActualDaysLabel").get<std::string>();
    invalid_days_message_ = config_json.at("InvalidDaysMessage").get<std::string>();
    no_records_ = config_json.at("NoRecords").get<std::string>();
}

// Getters
const std::string& PeriodTypConfig::get_base_font() const { return base_font_; }
const std::string& PeriodTypConfig::get_title_font() const { return title_font_; }
int PeriodTypConfig::get_base_font_size() const { return base_font_size_; }
int PeriodTypConfig::get_report_title_font_size() const { return report_title_font_size_; }
int PeriodTypConfig::get_category_title_font_size() const { return category_title_font_size_; }
double PeriodTypConfig::get_line_spacing_em() const { return line_spacing_em_; }
double PeriodTypConfig::get_margin_top_cm() const { return margin_top_cm_; }
double PeriodTypConfig::get_margin_bottom_cm() const { return margin_bottom_cm_; }
double PeriodTypConfig::get_margin_left_cm() const { return margin_left_cm_; }
double PeriodTypConfig::get_margin_right_cm() const { return margin_right_cm_; }
const std::string& PeriodTypConfig::get_title_prefix() const { return title_prefix_; }
const std::string& PeriodTypConfig::get_title_days_unit() const { return title_days_unit_; }
const std::string& PeriodTypConfig::get_title_date_separator() const { return title_date_separator_; }
const std::string& PeriodTypConfig::get_total_time_label() const { return total_time_label_; }
const std::string& PeriodTypConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& PeriodTypConfig::get_invalid_days_message() const { return invalid_days_message_; }
const std::string& PeriodTypConfig::get_no_records() const { return no_records_; }