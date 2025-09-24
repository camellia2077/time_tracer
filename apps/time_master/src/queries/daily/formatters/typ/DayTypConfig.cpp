// queries/daily/formatters/typ/DayTypConfig.cpp
#include "DayTypConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp"
#include <stdexcept>

DayTypConfig::DayTypConfig(const std::string& config_path) {
    load_config(config_path);
}

void DayTypConfig::load_config(const std::string& config_path) {
    nlohmann::json config_json = load_json_config(config_path, "Could not open Typst daily report config file: ");

    title_font_ = config_json.at("title_font").get<std::string>();
    base_font_ = config_json.at("base_font").get<std::string>();
    category_title_font_ = config_json.at("category_title_font").get<std::string>();
    base_font_size_ = config_json.at("base_font_size").get<int>();
    report_title_font_size_ = config_json.at("report_title_font_size").get<int>();
    category_title_font_size_ = config_json.at("category_title_font_size").get<int>();
    statistic_font_size_ = config_json.at("statistic_font_size").get<int>();
    statistic_title_font_size_ = config_json.at("statistic_title_font_size").get<int>();
    line_spacing_em_ = config_json.at("line_spacing_em").get<double>();
    keyword_colors_ = config_json.at("keyword_colors").get<std::map<std::string, std::string>>();
    title_prefix_ = config_json.at("title_prefix").get<std::string>();
    date_label_ = config_json.at("date_label").get<std::string>();
    total_time_label_ = config_json.at("total_time_label").get<std::string>();
    status_label_ = config_json.at("status_label").get<std::string>();
    sleep_label_ = config_json.at("sleep_label").get<std::string>();
    getup_time_label_ = config_json.at("getup_time_label").get<std::string>();
    remark_label_ = config_json.at("remark_label").get<std::string>();
    exercise_label_ = config_json.at("exercise_label").get<std::string>();
    no_records_ = config_json.at("no_records").get<std::string>();
    statistics_label_ = config_json.at("statistics_label").get<std::string>();
    all_activities_label_ = config_json.at("all_activities_label").get<std::string>();
    activity_remark_label_ = config_json.at("activity_remark_label").get<std::string>();
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
const std::string& DayTypConfig::get_title_font() const { return title_font_; }
const std::string& DayTypConfig::get_base_font() const { return base_font_; }
const std::string& DayTypConfig::get_category_title_font() const { return category_title_font_; }
int DayTypConfig::get_base_font_size() const { return base_font_size_; }
int DayTypConfig::get_report_title_font_size() const { return report_title_font_size_; }
int DayTypConfig::get_category_title_font_size() const { return category_title_font_size_; }
int DayTypConfig::get_statistic_font_size() const { return statistic_font_size_; }
int DayTypConfig::get_statistic_title_font_size() const { return statistic_title_font_size_; }
double DayTypConfig::get_line_spacing_em() const { return line_spacing_em_; }
const std::map<std::string, std::string>& DayTypConfig::get_keyword_colors() const { return keyword_colors_; }
const std::string& DayTypConfig::get_title_prefix() const { return title_prefix_; }
const std::string& DayTypConfig::get_date_label() const { return date_label_; }
const std::string& DayTypConfig::get_total_time_label() const { return total_time_label_; }
const std::string& DayTypConfig::get_status_label() const { return status_label_; }
const std::string& DayTypConfig::get_sleep_label() const { return sleep_label_; }
const std::string& DayTypConfig::get_getup_time_label() const { return getup_time_label_; }
const std::string& DayTypConfig::get_remark_label() const { return remark_label_; }
const std::string& DayTypConfig::get_exercise_label() const { return exercise_label_; }
const std::string& DayTypConfig::get_no_records() const { return no_records_; }
const std::string& DayTypConfig::get_statistics_label() const { return statistics_label_; }
const std::string& DayTypConfig::get_all_activities_label() const { return all_activities_label_; }
const std::string& DayTypConfig::get_activity_remark_label() const { return activity_remark_label_; }
const std::string& DayTypConfig::get_activity_connector() const { return activity_connector_; }
const std::map<std::string, StatisticItemConfig>& DayTypConfig::get_statistics_items() const {
    return statistics_items_;
}