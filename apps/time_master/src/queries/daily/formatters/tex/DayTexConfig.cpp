// queries/daily/formatters/tex/DayTexConfig.cpp
#include "DayTexConfig.hpp"
#include "queries/shared/utils/config/ConfigUtils.hpp"
#include <stdexcept>

DayTexConfig::DayTexConfig(const std::string& config_path) {
    load_config(config_path);
}

void DayTexConfig::load_config(const std::string& config_path) {
    nlohmann::json config_json = load_json_config(config_path, "Could not open DayTexConfig file: ");

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
    // --- [核心修改] 加载新配置 ---
    anaerobic_time_label_ = config_json.at("AnaerobicTimeLabel").get<std::string>();
    cardio_time_label_ = config_json.at("CardioTimeLabel").get<std::string>();
    grooming_time_label_ = config_json.at("GroomingTimeLabel").get<std::string>();
    activity_remark_label_ = config_json.at("ActivityRemarkLabel").get<std::string>();
    main_font_ = config_json.at("MainFont").get<std::string>();
    cjk_main_font_ = config_json.at("CJKMainFont").get<std::string>();
    base_font_size_ = config_json.at("BaseFontSize").get<int>();
    report_title_font_size_ = config_json.at("ReportTitleFontSize").get<int>();
    category_title_font_size_ = config_json.at("CategoryTitleFontSize").get<int>();
    margin_in_ = config_json.at("Margin_in").get<double>();
    list_top_sep_pt_ = config_json.at("ListTopSep_pt").get<double>();
    list_item_sep_ex_ = config_json.at("ListItemSep_ex").get<double>();
    keyword_colors_ = config_json.at("KeywordColors").get<std::map<std::string, std::string>>();
    activity_connector_ = config_json.at("ActivityConnector").get<std::string>();
}

// Getters
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
// --- [核心修改] 新增 Getter 实现 ---
const std::string& DayTexConfig::get_anaerobic_time_label() const { return anaerobic_time_label_; }
const std::string& DayTexConfig::get_cardio_time_label() const { return cardio_time_label_; }
const std::string& DayTexConfig::get_grooming_time_label() const { return grooming_time_label_; }
const std::string& DayTexConfig::get_activity_remark_label() const { return activity_remark_label_; }
const std::string& DayTexConfig::get_main_font() const { return main_font_; }
const std::string& DayTexConfig::get_cjk_main_font() const { return cjk_main_font_; }
int DayTexConfig::get_base_font_size() const { return base_font_size_; }
int DayTexConfig::get_report_title_font_size() const { return report_title_font_size_; }
int DayTexConfig::get_category_title_font_size() const { return category_title_font_size_; }
double DayTexConfig::get_margin_in() const { return margin_in_; }
double DayTexConfig::get_list_top_sep_pt() const { return list_top_sep_pt_; }
double DayTexConfig::get_list_item_sep_ex() const { return list_item_sep_ex_; }
const std::map<std::string, std::string>& DayTexConfig::get_keyword_colors() const { return keyword_colors_; }
const std::string& DayTexConfig::get_activity_connector() const { return activity_connector_; }