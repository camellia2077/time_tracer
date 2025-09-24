// queries/daily/formatters/tex/DayTexConfig.cpp
#include "DayTexConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp"
#include <stdexcept>

DayTexConfig::DayTexConfig(const std::string& config_path) {
    load_config(config_path);
}

void DayTexConfig::load_config(const std::string& config_path) {
    nlohmann::json config_json = load_json_config(config_path, "Could not open DayTexConfig file: ");

    report_title_ = config_json.at("report_title").get<std::string>();
    date_label_ = config_json.at("date_label").get<std::string>();
    total_time_label_ = config_json.at("total_time_label").get<std::string>();
    status_label_ = config_json.at("status_label").get<std::string>();
    sleep_label_ = config_json.at("sleep_label").get<std::string>();
    exercise_label_ = config_json.at("exercise_label").get<std::string>();
    getup_time_label_ = config_json.at("getup_time_label").get<std::string>();
    remark_label_ = config_json.at("remark_label").get<std::string>();
    no_records_message_ = config_json.at("no_records_message").get<std::string>();
    statistics_label_ = config_json.at("statistics_label").get<std::string>();
    all_activities_label_ = config_json.at("all_activities_label").get<std::string>();
    activity_remark_label_ = config_json.at("activity_remark_label").get<std::string>();
    main_font_ = config_json.at("main_font").get<std::string>();
    cjk_main_font_ = config_json.at("cjk_main_font").get<std::string>();
    base_font_size_ = config_json.at("base_font_size").get<int>();
    report_title_font_size_ = config_json.at("report_title_font_size").get<int>();
    category_title_font_size_ = config_json.at("category_title_font_size").get<int>();
    margin_in_ = config_json.at("margin_in").get<double>();
    list_top_sep_pt_ = config_json.at("list_top_sep_pt").get<double>();
    list_item_sep_ex_ = config_json.at("list_item_sep_ex").get<double>();
    keyword_colors_ = config_json.at("keyword_colors").get<std::map<std::string, std::string>>();
    activity_connector_ = config_json.at("activity_connector").get<std::string>();

    if (config_json.contains("statistics_items")) {
        for (auto& [key, value] : config_json["statistics_items"].items()) {
            statistics_items_[key] = {
                value.at("label").get<std::string>(),
                value.value("show", true)
            };
        }
    }
}

// Getters
const std::string& DayTexConfig::get_report_title() const { return report_title_; }
const std::string& DayTexConfig::get_date_label() const { return date_label_; }
const std::string& DayTexConfig::get_total_time_label() const { return total_time_label_; }
const std::string& DayTexConfig::get_status_label() const { return status_label_; }
const std::string& DayTexConfig::get_sleep_label() const { return sleep_label_; }
const std::string& DayTexConfig::get_exercise_label() const { return exercise_label_; }
const std::string& DayTexConfig::get_getup_time_label() const { return getup_time_label_; }
const std::string& DayTexConfig::get_remark_label() const { return remark_label_; }
const std::string& DayTexConfig::get_no_records_message() const { return no_records_message_; }
const std::string& DayTexConfig::get_statistics_label() const { return statistics_label_; }
const std::string& DayTexConfig::get_all_activities_label() const { return all_activities_label_; }
const std::string& DayTexConfig::get_activity_remark_label() const { return activity_remark_label_; }
const std::string& DayTexConfig::get_main_font() const { return main_font_; }
const std::string& DayTexConfig::get_cjk_main_font() const { return cjk_main_font_; }
int DayTexConfig::get_base_font_size() const { return base_font_size_; }
int DayTexConfig::get_report_title_font_size() const { return report_title_font_size_; }
int DayTexConfig::get_category_title_font_size() const { return category_title_font_size_; }
double DayTexConfig::get_margin_in() const { return margin_in_; }
double DayTexConfig::get_list_top_sep_pt() const { return list_top_sep_pt_; }
double DayTexConfig::get_list_item_sep_ex() const { return list_item_sep_ex_; }
const std::map<std::string, std::string>& DayTexConfig::get_keyword_colors() const { return keyword_colors_; }
const std::string& DayTexConfig::get_activity_connector() const { return activity_connector_; }
const std::map<std::string, StatisticItemConfig>& DayTexConfig::get_statistics_items() const {
    return statistics_items_;
}