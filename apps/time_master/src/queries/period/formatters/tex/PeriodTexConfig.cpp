// queries/period/formatters/tex/PeriodTexConfig.cpp
#include "PeriodTexConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp"
#include <stdexcept>

PeriodTexConfig::PeriodTexConfig(const std::string& config_path) {
    load_config(config_path);
}

void PeriodTexConfig::load_config(const std::string& config_path) {
    nlohmann::json config_json = load_json_config(config_path, "Could not open PeriodTexConfig file: ");

    report_title_prefix_ = config_json.at("report_title_prefix").get<std::string>();
    report_title_days_ = config_json.at("report_title_days").get<std::string>();
    report_title_date_separator_ = config_json.at("report_title_date_separator").get<std::string>();
    total_time_label_ = config_json.at("total_time_label").get<std::string>();
    actual_days_label_ = config_json.at("actual_days_label").get<std::string>();
    no_records_message_ = config_json.at("no_records_message").get<std::string>();
    invalid_days_message_ = config_json.at("invalid_days_message").get<std::string>();
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
const std::string& PeriodTexConfig::get_report_title_prefix() const { return report_title_prefix_; }
const std::string& PeriodTexConfig::get_report_title_days() const { return report_title_days_; }
const std::string& PeriodTexConfig::get_report_title_date_separator() const { return report_title_date_separator_; }
const std::string& PeriodTexConfig::get_total_time_label() const { return total_time_label_; }
const std::string& PeriodTexConfig::get_actual_days_label() const { return actual_days_label_; }
const std::string& PeriodTexConfig::get_no_records_message() const { return no_records_message_; }
const std::string& PeriodTexConfig::get_invalid_days_message() const { return invalid_days_message_; }
const std::string& PeriodTexConfig::get_main_font() const { return main_font_; }
const std::string& PeriodTexConfig::get_cjk_main_font() const { return cjk_main_font_; }
int PeriodTexConfig::get_base_font_size() const { return base_font_size_; }
int PeriodTexConfig::get_report_title_font_size() const { return report_title_font_size_; }
int PeriodTexConfig::get_category_title_font_size() const { return category_title_font_size_; }
double PeriodTexConfig::get_margin_in() const { return margin_in_; }
double PeriodTexConfig::get_list_top_sep_pt() const { return list_top_sep_pt_; }
double PeriodTexConfig::get_list_item_sep_ex() const { return list_item_sep_ex_; }