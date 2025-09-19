// queries/period/formatters/tex/PeriodTexConfig.cpp
#include "PeriodTexConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp"
#include <stdexcept>

PeriodTexConfig::PeriodTexConfig(const std::string& config_path) {
    load_config(config_path);
}

void PeriodTexConfig::load_config(const std::string& config_path) {
    nlohmann::json config_json = load_json_config(config_path, "Could not open PeriodTexConfig file: ");

    report_title_prefix_ = config_json.at("ReportTitlePrefix").get<std::string>();
    report_title_days_ = config_json.at("ReportTitleDays").get<std::string>();
    report_title_date_separator_ = config_json.at("ReportTitleDateSeparator").get<std::string>();
    total_time_label_ = config_json.at("TotalTimeLabel").get<std::string>();
    actual_days_label_ = config_json.at("ActualDaysLabel").get<std::string>();
    no_records_message_ = config_json.at("NoRecordsMessage").get<std::string>();
    invalid_days_message_ = config_json.at("InvalidDaysMessage").get<std::string>();
    main_font_ = config_json.at("MainFont").get<std::string>();
    cjk_main_font_ = config_json.at("CJKMainFont").get<std::string>();
    font_size_ = config_json.at("FontSize").get<int>();
    margin_in_ = config_json.at("Margin_in").get<double>();
    list_top_sep_pt_ = config_json.at("ListTopSep_pt").get<double>();
    list_item_sep_ex_ = config_json.at("ListItemSep_ex").get<double>();
}

// Getters
const std::string& PeriodTexConfig::get_report_title_prefix() const { return report_title_prefix_; }
const std::string& PeriodTexConfig::get_report_title_days() const { return report_title_days_; }
const std::string& PeriodTexConfig::get_report_title_date_separator() const { return report_title_date_separator_; }
const std::string& PeriodTexConfig::get_total_time_label() const { return total_time_label_; }
const std::string& PeriodTexConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& PeriodTexConfig::get_no_records_message() const { return no_records_message_; }
const std::string& PeriodTexConfig::get_invalid_days_message() const { return invalid_days_message_; }
const std::string& PeriodTexConfig::get_main_font() const { return main_font_; }
const std::string& PeriodTexConfig::get_cjk_main_font() const { return cjk_main_font_; }
int PeriodTexConfig::get_font_size() const { return font_size_; }
double PeriodTexConfig::get_margin_in() const { return margin_in_; }
double PeriodTexConfig::get_list_top_sep_pt() const { return list_top_sep_pt_; }
double PeriodTexConfig::get_list_item_sep_ex() const { return list_item_sep_ex_; }