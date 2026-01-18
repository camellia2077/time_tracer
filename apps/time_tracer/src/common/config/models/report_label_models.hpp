// common/config/models/report_label_models.hpp
#ifndef COMMON_CONFIG_MODELS_REPORT_LABEL_MODELS_HPP_
#define COMMON_CONFIG_MODELS_REPORT_LABEL_MODELS_HPP_

#include <string>

struct DailyReportLabels {
    std::string report_title_prefix;
    std::string date_label;
    std::string total_time_label;
    std::string status_label;
    std::string sleep_label;
    std::string exercise_label;
    std::string getup_time_label;
    std::string remark_label;
    std::string no_records_message;
    
    std::string statistics_label;
    std::string all_activities_label;
    std::string activity_remark_label;
    std::string project_breakdown_label;
    std::string activity_connector;
};

struct MonthlyReportLabels {
    std::string report_title;
    std::string total_time_label;
    std::string actual_days_label;
    std::string no_records_message;
};

struct PeriodReportLabels {
    std::string report_title_prefix;
    std::string report_title_days;
    std::string report_title_date_separator;
    std::string total_time_label;
    std::string actual_days_label;
    std::string no_records_message;
    std::string invalid_days_message;
};

#endif // COMMON_CONFIG_MODELS_REPORT_LABEL_MODELS_HPP_