// infrastructure/config/models/report_label_models.hpp
#ifndef INFRASTRUCTURE_CONFIG_MODELS_REPORT_LABEL_MODELS_H_
#define INFRASTRUCTURE_CONFIG_MODELS_REPORT_LABEL_MODELS_H_

#include <string>

struct DailyReportLabels {
  std::string report_title_prefix;
  std::string report_title;
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

struct RangeReportLabels {
  std::string report_title;
  std::string title_template;
  std::string total_time_label;
  std::string actual_days_label;
  std::string status_days_label;
  std::string sleep_days_label;
  std::string exercise_days_label;
  std::string cardio_days_label;
  std::string anaerobic_days_label;
  std::string no_records_message;
  std::string invalid_format_message;
  std::string invalid_range_message;
  std::string project_breakdown_label;
};

using MonthlyReportLabels = RangeReportLabels;
using PeriodReportLabels = RangeReportLabels;
using WeeklyReportLabels = RangeReportLabels;
using YearlyReportLabels = RangeReportLabels;

#endif  // INFRASTRUCTURE_CONFIG_MODELS_REPORT_LABEL_MODELS_H_
