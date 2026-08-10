// infra/config/models/insights_label_models.hpp
#ifndef INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_LABEL_MODELS_H_
#define INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_LABEL_MODELS_H_

#include <string>

struct DailyInsightsLabels {
  std::string insights_title_prefix;
  std::string insights_title;
  std::string date_label;
  std::string total_time_label;
  std::string activity_count_label;
  std::string getup_time_label;
  std::string remark_label;
  std::string no_records_message;

  std::string statistics_label;
  std::string all_activities_label;
  std::string activity_remark_label;
  std::string project_breakdown_label;
  std::string activity_connector;
};

struct RangeInsightsLabels {
  std::string insights_title;
  std::string title_template;
  std::string total_time_label;
  std::string activity_count_label;
  std::string actual_days_label;
  std::string status_days_label;
  std::string exercise_days_label;
  std::string cardio_days_label;
  std::string anaerobic_days_label;
  std::string no_records_message;
  std::string invalid_format_message;
  std::string invalid_range_message;
  std::string project_breakdown_label;
};

using MonthlyInsightsLabels = RangeInsightsLabels;
using PeriodInsightsLabels = RangeInsightsLabels;
using WeeklyInsightsLabels = RangeInsightsLabels;
using YearlyInsightsLabels = RangeInsightsLabels;

#endif  // INFRASTRUCTURE_CONFIG_MODELS_INSIGHTS_LABEL_MODELS_H_
