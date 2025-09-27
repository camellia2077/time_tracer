// reports/daily/formatters/md/DayMdConfig.cpp
#include "DayMdConfig.hpp"
#include "reports/shared/utils/config/ConfigUtils.hpp"
#include <stdexcept>

DayMdConfig::DayMdConfig(const std::string& config_path) {
    load_config(config_path);
}

void DayMdConfig::load_config(const std::string& config_path) {
    nlohmann::json config_json = load_json_config(config_path, "Could not open DayMdConfig file: ");

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
    
    // [修改] 加载新的 statistics_items 结构
    if (config_json.contains("statistics_items")) {
        for (auto& [key, value] : config_json["statistics_items"].items()) {
            statistics_items_[key] = {
                value.at("label").get<std::string>(),
                value.value("show", true)
            };
        }
    }
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
const std::string& DayMdConfig::get_activity_remark_label() const { return activity_remark_label_; }
const std::string& DayMdConfig::get_activity_connector() const { return activity_connector_; }
const std::map<std::string, StatisticItemConfig>& DayMdConfig::get_statistics_items() const {
    return statistics_items_;
}