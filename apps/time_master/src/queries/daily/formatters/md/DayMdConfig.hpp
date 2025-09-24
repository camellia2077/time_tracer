// queries/daily/formatters/md/DayMdConfig.hpp
#ifndef DAY_MD_CONFIG_HPP
#define DAY_MD_CONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>

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
    const std::string& get_sleep_time_label() const;
    const std::string& get_anaerobic_time_label() const;
    const std::string& get_cardio_time_label() const;
    const std::string& get_grooming_time_label() const;
    const std::string& get_activity_remark_label() const;
    const std::string& get_activity_connector() const; 
    // --- [新增 Getter] ---
    const std::string& get_recreation_time_label() const;
    const std::string& get_zhihu_time_label() const;
    const std::string& get_bilibili_time_label() const;
    const std::string& get_douyin_time_label() const;

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
    std::string sleep_time_label_;
    std::string anaerobic_time_label_;
    std::string cardio_time_label_;
    std::string grooming_time_label_;
    std::string activity_remark_label_;
    std::string activity_connector_; 
    // --- [新增成员变量] ---
    std::string recreation_time_label_;
    std::string zhihu_time_label_;
    std::string bilibili_time_label_;
    std::string douyin_time_label_;
};

#endif // DAY_MD_CONFIG_HPP