// infra/insights/monthly/common/month_base_config.cpp
#include "infra/insights/monthly/common/month_base_config.hpp"

MonthBaseConfig::MonthBaseConfig(const MonthlyInsightsLabels& labels) {
  LoadBaseConfig(labels);
}

void MonthBaseConfig::LoadBaseConfig(const MonthlyInsightsLabels& labels) {
  actual_days_label_ = labels.actual_days_label;
  total_time_label_ = labels.total_time_label;
  activity_count_label_ = labels.activity_count_label;
  status_count_unit_ = labels.status_count_unit;
  custom_section_label_ = labels.custom_section_label;
  summary_section_label_ = labels.summary_section_label;
  period_label_ = labels.period_label;
  no_records_message_ = labels.no_records_message;
  invalid_format_message_ = labels.invalid_format_message;
  project_breakdown_label_ = labels.project_breakdown_label;
}

auto MonthBaseConfig::GetActualDaysLabel() const -> const std::string& {
  return actual_days_label_;
}
auto MonthBaseConfig::GetTotalTimeLabel() const -> const std::string& {
  return total_time_label_;
}
auto MonthBaseConfig::GetActivityCountLabel() const -> const std::string& {
  return activity_count_label_;
}
auto MonthBaseConfig::GetStatusCountUnit() const -> const std::string& {
  return status_count_unit_;
}
auto MonthBaseConfig::GetCustomSectionLabel() const -> const std::string& {
  return custom_section_label_;
}
auto MonthBaseConfig::GetSummarySectionLabel() const -> const std::string& {
  return summary_section_label_;
}
auto MonthBaseConfig::GetPeriodLabel() const -> const std::string& {
  return period_label_;
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
