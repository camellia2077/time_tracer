// queries/monthly/formatters/tex/MonthTexConfig.cpp
#include "MonthTexConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp"
#include <stdexcept>

MonthTexConfig::MonthTexConfig(const std::string& config_path) {
    load_config(config_path);
}

void MonthTexConfig::load_config(const std::string& config_path) {
    nlohmann::json config_json = load_json_config(config_path, "Could not open MonthTexConfig file: ");

    report_title_ = config_json.at("report_title").get<std::string>();
    actual_days_label_ = config_json.at("actual_days_label").get<std::string>();
    total_time_label_ = config_json.at("total_time_label").get<std::string>();
    no_records_message_ = config_json.at("no_records_message").get<std::string>();
    invalid_format_message_ = config_json.at("invalid_format_message").get<std::string>();
    main_font_ = config_json.at("main_font").get<std::string>();
    cjk_main_font_ = config_json.at("cjk_main_font").get<std::string>();
    base_font_size_ = config_json.at("base_font_size").get<int>();
    report_title_font_size_ = config_json.at("report_title_font_size").get<int>();
    category_title_font_size_ = config_json.at("category_title_font_size").get<int>();
    margin_in_ = config_json.at("margin_in").get<double>();
    list_top_sep_pt_ = config_json.at("list_top_sep_pt").get<double>();
    list_item_sep_ex_ = config_json.at("list_item_sep_ex").get<double>();
}

// Getters
const std::string& MonthTexConfig::get_report_title() const { return report_title_; }
const std::string& MonthTexConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& MonthTexConfig::get_total_time_label() const { return total_time_label_; }
const std::string& MonthTexConfig::get_no_records_message() const { return no_records_message_; }
const std::string& MonthTexConfig::get_invalid_format_message() const { return invalid_format_message_; }
const std::string& MonthTexConfig::get_main_font() const { return main_font_; }
const std::string& MonthTexConfig::get_cjk_main_font() const { return cjk_main_font_; }
int MonthTexConfig::get_base_font_size() const { return base_font_size_; }
int MonthTexConfig::get_report_title_font_size() const { return report_title_font_size_; }
int MonthTexConfig::get_category_title_font_size() const { return category_title_font_size_; }
double MonthTexConfig::get_margin_in() const { return margin_in_; }
double MonthTexConfig::get_list_top_sep_pt() const { return list_top_sep_pt_; }
double MonthTexConfig::get_list_item_sep_ex() const { return list_item_sep_ex_; }