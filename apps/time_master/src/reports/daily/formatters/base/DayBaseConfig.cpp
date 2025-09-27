// reports/daily/formatters/base/DayBaseConfig.cpp
#include "DayBaseConfig.hpp"
#include <stdexcept>

DayBaseConfig::DayBaseConfig(const std::string& config_path) {
    // 加载JSON文件，并存储以供子类使用
    config_json_ = load_json_config(config_path, "Could not open Day config file: ");
    load_base_config();
}

void DayBaseConfig::load_base_config() {
    // 从json对象中加载通用配置
    // [修正] 使用 .value() 代替 .at()，以允许 title_prefix 键是可选的
    title_prefix_ = config_json_.value("title_prefix", ""); 

    date_label_ = config_json_.at("date_label").get<std::string>();
    total_time_label_ = config_json_.at("total_time_label").get<std::string>();
    status_label_ = config_json_.at("status_label").get<std::string>();
    sleep_label_ = config_json_.at("sleep_label").get<std::string>();
    getup_time_label_ = config_json_.at("getup_time_label").get<std::string>();
    remark_label_ = config_json_.at("remark_label").get<std::string>();
    exercise_label_ = config_json_.at("exercise_label").get<std::string>();
    
    // 兼容 "no_records" 和 "no_records_message" 两种键名
    if (config_json_.contains("no_records")) {
        no_records_ = config_json_.at("no_records").get<std::string>();
    } else if (config_json_.contains("no_records_message")) {
        no_records_ = config_json_.at("no_records_message").get<std::string>();
    }

    statistics_label_ = config_json_.at("statistics_label").get<std::string>();
    all_activities_label_ = config_json_.at("all_activities_label").get<std::string>();
    activity_remark_label_ = config_json_.at("activity_remark_label").get<std::string>();
    activity_connector_ = config_json_.at("activity_connector").get<std::string>();
    
    if (config_json_.contains("statistics_items")) {
        for (auto& [key, value] : config_json_["statistics_items"].items()) {
            statistics_items_[key] = {
                value.at("label").get<std::string>(),
                value.value("show", true)
            };
        }
    }
}

// --- Getters 实现 ---
const std::string& DayBaseConfig::get_title_prefix() const { return title_prefix_; }
const std::string& DayBaseConfig::get_date_label() const { return date_label_; }
const std::string& DayBaseConfig::get_total_time_label() const { return total_time_label_; }
const std::string& DayBaseConfig::get_status_label() const { return status_label_; }
const std::string& DayBaseConfig::get_sleep_label() const { return sleep_label_; }
const std::string& DayBaseConfig::get_getup_time_label() const { return getup_time_label_; }
const std::string& DayBaseConfig::get_remark_label() const { return remark_label_; }
const std::string& DayBaseConfig::get_exercise_label() const { return exercise_label_; }
const std::string& DayBaseConfig::get_no_records() const { return no_records_; }
const std::string& DayBaseConfig::get_statistics_label() const { return statistics_label_; }
const std::string& DayBaseConfig::get_all_activities_label() const { return all_activities_label_; }
const std::string& DayBaseConfig::get_activity_remark_label() const { return activity_remark_label_; }
const std::string& DayBaseConfig::get_activity_connector() const { return activity_connector_; }
const std::map<std::string, StatisticItemConfig>& DayBaseConfig::get_statistics_items() const {
    return statistics_items_;
}