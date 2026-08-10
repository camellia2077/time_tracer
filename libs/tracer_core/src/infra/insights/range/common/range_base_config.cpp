// infra/insights/range/common/range_base_config.cpp
#include "infra/insights/range/common/range_base_config.hpp"

RangeBaseConfig::RangeBaseConfig(const RangeInsightsLabels& labels) {
  LoadBaseConfig(labels);
}

void RangeBaseConfig::LoadBaseConfig(const RangeInsightsLabels& labels) {
  total_time_label_ = labels.total_time_label;
  activity_count_label_ = labels.activity_count_label;
  status_count_unit_ = labels.status_count_unit;
  custom_section_label_ = labels.custom_section_label;
  summary_section_label_ = labels.summary_section_label;
  period_label_ = labels.period_label;
  actual_days_label_ = labels.actual_days_label;
  no_records_message_ = labels.no_records_message;
  invalid_range_message_ = labels.invalid_range_message;
  project_breakdown_label_ = labels.project_breakdown_label;
}

auto RangeBaseConfig::GetTotalTimeLabel() const -> const std::string& {
  return total_time_label_;
}
auto RangeBaseConfig::GetActivityCountLabel() const -> const std::string& {
  return activity_count_label_;
}
auto RangeBaseConfig::GetStatusCountUnit() const -> const std::string& {
  return status_count_unit_;
}
auto RangeBaseConfig::GetCustomSectionLabel() const -> const std::string& {
  return custom_section_label_;
}
auto RangeBaseConfig::GetSummarySectionLabel() const -> const std::string& {
  return summary_section_label_;
}
auto RangeBaseConfig::GetPeriodLabel() const -> const std::string& {
  return period_label_;
}

auto RangeBaseConfig::GetActualDaysLabel() const -> const std::string& {
  return actual_days_label_;
}

auto RangeBaseConfig::GetNoRecordsMessage() const -> const std::string& {
  return no_records_message_;
}

auto RangeBaseConfig::GetInvalidRangeMessage() const -> const std::string& {
  return invalid_range_message_;
}

auto RangeBaseConfig::GetProjectBreakdownLabel() const -> const std::string& {
  return project_breakdown_label_;
}
