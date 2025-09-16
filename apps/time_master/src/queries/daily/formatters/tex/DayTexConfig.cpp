#include "DayTexConfig.hpp"
#include <fstream>
#include <stdexcept>

DayTexConfig::DayTexConfig(const std::string& config_path) {
    load_config(config_path);
}

void DayTexConfig::load_config(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open DayTexConfig file: " + config_path);
    }
    nlohmann::json config_json;
    config_file >> config_json;

    // ... (existing JSON loading remains the same)
    report_title_ = config_json.at("ReportTitle").get<std::string>();
    date_label_ = config_json.at("DateLabel").get<std::string>();
    total_time_label_ = config_json.at("TotalTimeLabel").get<std::string>();
    status_label_ = config_json.at("StatusLabel").get<std::string>();
    sleep_label_ = config_json.at("SleepLabel").get<std::string>();
    exercise_label_ = config_json.at("ExerciseLabel").get<std::string>();
    getup_time_label_ = config_json.at("GetupTimeLabel").get<std::string>();
    remark_label_ = config_json.at("RemarkLabel").get<std::string>();
    no_records_message_ = config_json.at("NoRecordsMessage").get<std::string>();
    statistics_label_ = config_json.at("StatisticsLabel").get<std::string>();
    all_activities_label_ = config_json.at("AllActivitiesLabel").get<std::string>();
    sleep_time_label_ = config_json.at("SleepTimeLabel").get<std::string>();
    activity_remark_label_ = config_json.at("ActivityRemarkLabel").get<std::string>();
    compact_list_options_ = config_json.at("CompactListOptions").get<std::string>();
    main_font_ = config_json.at("MainFont").get<std::string>();
    cjk_main_font_ = config_json.at("CJKMainFont").get<std::string>();
    
    // [New] Load the keyword colors
    keyword_colors_ = config_json.at("KeywordColors").get<std::map<std::string, std::string>>();
}

// ... (existing getters remain the same)
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
const std::string& DayTexConfig::get_sleep_time_label() const { return sleep_time_label_; }
const std::string& DayTexConfig::get_activity_remark_label() const { return activity_remark_label_; }
const std::string& DayTexConfig::get_compact_list_options() const { return compact_list_options_; }
const std::string& DayTexConfig::get_main_font() const { return main_font_; }
const std::string& DayTexConfig::get_cjk_main_font() const { return cjk_main_font_; }

// [New] Getter for keyword colors
const std::map<std::string, std::string>& DayTexConfig::get_keyword_colors() const { return keyword_colors_; }