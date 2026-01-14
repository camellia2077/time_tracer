// reports/daily/common/DayBaseConfig.cpp
#include "DayBaseConfig.hpp"
#include <stdexcept>

// 递归解析统计项的辅助函数
static std::vector<StatisticItemConfig> parse_statistics_items_recursive(const nlohmann::json& json_array) {
    std::vector<StatisticItemConfig> items;
    if (!json_array.is_array()) {
        return items;
    }

    for (const auto& item_json : json_array) {
        StatisticItemConfig item;
        item.label = item_json.at("label").get<std::string>();
        item.db_column = item_json.value("db_column", ""); 
        item.show = item_json.value("show", true);

        if (item_json.contains("sub_items")) {
            item.sub_items = parse_statistics_items_recursive(item_json["sub_items"]);
        }

        items.push_back(item);
    }
    return items;
}

// [修改] 构造函数直接接收 JSON，不再进行 IO
DayBaseConfig::DayBaseConfig(const nlohmann::json& config) 
    : config_json_(config) 
{
    load_base_config();
}

void DayBaseConfig::load_base_config() {
    // 从 config_json_ 中加载数据
    title_prefix_ = config_json_.value("title_prefix", ""); 
    date_label_ = config_json_.at("date_label").get<std::string>();
    total_time_label_ = config_json_.at("total_time_label").get<std::string>();
    status_label_ = config_json_.at("status_label").get<std::string>();
    sleep_label_ = config_json_.at("sleep_label").get<std::string>();
    getup_time_label_ = config_json_.at("getup_time_label").get<std::string>();
    remark_label_ = config_json_.at("remark_label").get<std::string>();
    exercise_label_ = config_json_.at("exercise_label").get<std::string>();
    
    no_records_ = config_json_.at("no_records_message").get<std::string>();

    statistics_label_ = config_json_.at("statistics_label").get<std::string>();
    all_activities_label_ = config_json_.at("all_activities_label").get<std::string>();
    activity_remark_label_ = config_json_.at("activity_remark_label").get<std::string>();
    activity_connector_ = config_json_.at("activity_connector").get<std::string>();

    project_breakdown_label_ = config_json_.value("project_breakdown_label", "Project Breakdown");
    
    if (config_json_.contains("statistics_items")) {
        statistics_items_ = parse_statistics_items_recursive(config_json_["statistics_items"]);
    }
}

// Getters 实现保持不变
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
const std::vector<StatisticItemConfig>& DayBaseConfig::get_statistics_items() const { return statistics_items_; }
const std::string& DayBaseConfig::get_project_breakdown_label() const { return project_breakdown_label_; }
