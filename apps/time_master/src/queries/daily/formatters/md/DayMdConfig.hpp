// queries/daily/formatters/md/DayMdConfig.hpp
#ifndef DAY_MD_CONFIG_HPP
#define DAY_MD_CONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>
#include <map>

// [新增] 用于存储单个统计项配置的结构体
struct StatisticItemConfig {
    std::string label;
    bool show = true;
};

class DayMdConfig {
public:
    explicit DayMdConfig(const std::string& config_path);

    const std::string& get_title_prefix() const;
    const std::string& get_date_label() const;
    const std::string& get_total_time_label() const;
    const std::string& get_status_label() const;
    const std::string& get_sleep_label() const;
    const std::string& get_getup_time_label() const;
    const std::string& get_remark_label() const;
    const std::string& get_exercise_label() const;
    const std::string& get_no_records() const;
    const std::string& get_statistics_label() const;
    const std::string& get_all_activities_label() const;
    const std::string& get_activity_remark_label() const;
    const std::string& get_activity_connector() const;
    
    // [修改] 提供访问所有统计项配置的接口
    const std::map<std::string, StatisticItemConfig>& get_statistics_items() const;

private:
    void load_config(const std::string& config_path);

    std::string title_prefix_;
    std::string date_label_;
    std::string total_time_label_;
    std::string status_label_;
    std::string sleep_label_;
    std::string getup_time_label_;
    std::string remark_label_;
    std::string exercise_label_;
    std::string no_records_;
    std::string statistics_label_;
    std::string all_activities_label_;
    std::string activity_remark_label_;
    std::string activity_connector_; 

    // [修改] 使用 map 存储所有统计项的配置
    std::map<std::string, StatisticItemConfig> statistics_items_;
};

#endif // DAY_MD_CONFIG_HPP