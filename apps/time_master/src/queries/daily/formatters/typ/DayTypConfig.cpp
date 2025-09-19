// queries/daily/formatters/typ/DayTypConfig.cpp
#include "DayTypConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp"
#include <stdexcept>

DayTypConfig::DayTypConfig(const std::string& config_path) {
    load_config(config_path);
}

void DayTypConfig::load_config(const std::string& config_path) {
    nlohmann::json config_json = load_json_config(config_path, "Could not open Typst daily report config file: ");

    title_font_ = config_json.at("TitleFont").get<std::string>();
    content_font_ = config_json.at("ContentFont").get<std::string>();
    title_font_size_ = config_json.at("TitleFontSize").get<int>();
    line_spacing_ = config_json.at("LineSpacing").get<std::string>();
    keyword_colors_ = config_json.at("KeywordColors").get<std::map<std::string, std::string>>();
    title_prefix_ = config_json.at("TitlePrefix").get<std::string>();
    date_label_ = config_json.at("DateLabel").get<std::string>();
    total_time_label_ = config_json.at("TotalTimeLabel").get<std::string>();
    status_label_ = config_json.at("StatusLabel").get<std::string>();
    sleep_label_ = config_json.at("SleepLabel").get<std::string>();
    getup_time_label_ = config_json.at("GetupTimeLabel").get<std::string>();
    remark_label_ = config_json.at("RemarkLabel").get<std::string>();
    exercise_label_ = config_json.at("ExerciseLabel").get<std::string>();
    no_records_ = config_json.at("NoRecords").get<std::string>();
    statistics_label_ = config_json.at("StatisticsLabel").get<std::string>();
    all_activities_label_ = config_json.at("AllActivitiesLabel").get<std::string>();
    sleep_time_label_ = config_json.at("SleepTimeLabel").get<std::string>();
    activity_remark_label_ = config_json.at("ActivityRemarkLabel").get<std::string>();
    activity_connector_ = config_json.at("ActivityConnector").get<std::string>(); // [ADD THIS]
}

const std::string& DayTypConfig::get_title_font() const { return title_font_; }
const std::string& DayTypConfig::get_content_font() const { return content_font_; }
int DayTypConfig::get_title_font_size() const { return title_font_size_; }
const std::string& DayTypConfig::get_line_spacing() const { return line_spacing_; }
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
const std::string& DayTypConfig::get_sleep_time_label() const { return sleep_time_label_; }
const std::string& DayTypConfig::get_activity_remark_label() const { return activity_remark_label_; }
const std::string& DayTypConfig::get_activity_connector() const { return activity_connector_; } // [ADD THIS]