// queries/daily/formatters/md/DayMdConfig.cpp
#include "DayMdConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp"
#include <stdexcept>

DayMdConfig::DayMdConfig(const std::string& config_path) {
    load_config(config_path);
}

void DayMdConfig::load_config(const std::string& config_path) {
    nlohmann::json config_json = load_json_config(config_path, "Could not open DayMdConfig file: ");

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
    // --- [核心修改] 加载新配置 ---
    anaerobic_time_label_ = config_json.at("AnaerobicTimeLabel").get<std::string>();
    cardio_time_label_ = config_json.at("CardioTimeLabel").get<std::string>();
    grooming_time_label_ = config_json.at("GroomingTimeLabel").get<std::string>();
    activity_remark_label_ = config_json.at("ActivityRemarkLabel").get<std::string>();
    activity_connector_ = config_json.at("ActivityConnector").get<std::string>(); 
}

const std::string& DayMdConfig::get_title_prefix() const { return title_prefix_; }
const std::string& DayMdConfig::get_date_label() const { return date_label_; }
const std::string& DayMdConfig::get_total_time_label() const { return total_time_label_; }
const std::string& DayMdConfig::get_status_label() const { return status_label_; }
const std::string& DayMdConfig::get_sleep_label() const { return sleep_label_; }
const std::string& DayMdConfig::get_getup_time_label() const { return getup_time_label_; }
const std::string& DayMdConfig::get_remark_label() const { return remark_label_; }
const std::string& DayMdConfig::get_exercise_label() const { return exercise_label_; }
const std::string& DayMdConfig::get_no_records() const { return no_records_; }
const std::string& DayMdConfig::get_statistics_label() const { return statistics_label_; }
const std::string& DayMdConfig::get_all_activities_label() const { return all_activities_label_; }
const std::string& DayMdConfig::get_sleep_time_label() const { return sleep_time_label_; }
// --- [核心修改] 新增 Getter ---
const std::string& DayMdConfig::get_anaerobic_time_label() const { return anaerobic_time_label_; }
const std::string& DayMdConfig::get_cardio_time_label() const { return cardio_time_label_; }
const std::string& DayMdConfig::get_grooming_time_label() const { return grooming_time_label_; }
const std::string& DayMdConfig::get_activity_remark_label() const { return activity_remark_label_; }
const std::string& DayMdConfig::get_activity_connector() const { return activity_connector_; }