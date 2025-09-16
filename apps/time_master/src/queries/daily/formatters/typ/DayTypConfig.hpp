#ifndef DAY_TYP_CONFIG_HPP
#define DAY_TYP_CONFIG_HPP

#include <string>
#include <map>
#include <nlohmann/json.hpp>

class DayTypConfig {
public:
    DayTypConfig(const std::string& config_path);

    const std::string& get_title_font() const;
    const std::string& get_content_font() const;
    int get_title_font_size() const;
    const std::string& get_line_spacing() const;
    const std::map<std::string, std::string>& get_keyword_colors() const;
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
    const std::string& get_activity_remark_label() const;
    const std::string& get_activity_connector() const; // [ADD THIS]

private:
    void load_config(const std::string& config_path);

    std::string title_font_;
    std::string content_font_;
    int title_font_size_;
    std::string line_spacing_;
    std::map<std::string, std::string> keyword_colors_;
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
    std::string activity_remark_label_;
    std::string activity_connector_; // [ADD THIS]
};

#endif // DAY_TYP_CONFIG_HPP