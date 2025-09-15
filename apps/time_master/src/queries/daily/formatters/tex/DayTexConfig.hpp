#ifndef DAY_TEX_CONFIG_HPP
#define DAY_TEX_CONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>

class DayTexConfig {
public:
    explicit DayTexConfig(const std::string& config_path);

    const std::string& get_report_title() const;
    const std::string& get_date_label() const;
    const std::string& get_total_time_label() const;
    const std::string& get_status_label() const;
    const std::string& get_sleep_label() const;
    const std::string& get_exercise_label() const;
    const std::string& get_getup_time_label() const;
    const std::string& get_remark_label() const;
    const std::string& get_no_records_message() const;
    const std::string& get_statistics_label() const;
    const std::string& get_all_activities_label() const;
    const std::string& get_sleep_time_label() const;
    const std::string& get_activity_remark_label() const;
    const std::string& get_compact_list_options() const;

private:
    void load_config(const std::string& config_path);

    std::string report_title_;
    std::string date_label_;
    std::string total_time_label_;
    std::string status_label_;
    std::string sleep_label_;
    std::string exercise_label_;
    std::string getup_time_label_;
    std::string remark_label_;
    std::string no_records_message_;
    std::string statistics_label_;
    std::string all_activities_label_;
    std::string sleep_time_label_;
    std::string activity_remark_label_;
    std::string compact_list_options_;
};

#endif // DAY_TEX_CONFIG_HPP