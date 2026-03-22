// infra/reports/monthly/common/month_base_config.cpp
#include "infra/reports/monthly/common/month_base_config.hpp"

MonthBaseConfig::MonthBaseConfig(const MonthlyReportLabels& labels) {
  LoadBaseConfig(labels);
}

void MonthBaseConfig::LoadBaseConfig(const MonthlyReportLabels& labels) {
  report_title_ = labels.report_title;
  title_template_ = labels.title_template;
  actual_days_label_ = labels.actual_days_label;
  status_days_label_ = labels.status_days_label;
  wake_anchor_days_label_ = labels.wake_anchor_days_label;
  exercise_days_label_ = labels.exercise_days_label;
  cardio_days_label_ = labels.cardio_days_label;
  anaerobic_days_label_ = labels.anaerobic_days_label;
  total_time_label_ = labels.total_time_label;
  no_records_message_ = labels.no_records_message;
  invalid_format_message_ = labels.invalid_format_message;
  project_breakdown_label_ = labels.project_breakdown_label;
}

auto MonthBaseConfig::GetReportTitle() const -> const std::string& {
  return report_title_;
}
auto MonthBaseConfig::GetTitleTemplate() const -> const std::string& {
  return title_template_;
}
auto MonthBaseConfig::GetActualDaysLabel() const -> const std::string& {
  return actual_days_label_;
}
auto MonthBaseConfig::GetStatusDaysLabel() const -> const std::string& {
  return status_days_label_;
}
auto MonthBaseConfig::GetWakeAnchorDaysLabel() const -> const std::string& {
  return wake_anchor_days_label_;
}
auto MonthBaseConfig::GetExerciseDaysLabel() const -> const std::string& {
  return exercise_days_label_;
}
auto MonthBaseConfig::GetCardioDaysLabel() const -> const std::string& {
  return cardio_days_label_;
}
auto MonthBaseConfig::GetAnaerobicDaysLabel() const -> const std::string& {
  return anaerobic_days_label_;
}
auto MonthBaseConfig::GetTotalTimeLabel() const -> const std::string& {
  return total_time_label_;
}
auto MonthBaseConfig::GetNoRecordsMessage() const -> const std::string& {
  return no_records_message_;
}
auto MonthBaseConfig::GetInvalidFormatMessage() const -> const std::string& {
  return invalid_format_message_;
}
auto MonthBaseConfig::GetProjectBreakdownLabel() const -> const std::string& {
  return project_breakdown_label_;
}
