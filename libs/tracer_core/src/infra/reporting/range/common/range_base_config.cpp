// infra/reporting/range/common/range_base_config.cpp
#include "infra/reporting/range/common/range_base_config.hpp"

RangeBaseConfig::RangeBaseConfig(const RangeReportLabels& labels) {
  LoadBaseConfig(labels);
}

void RangeBaseConfig::LoadBaseConfig(const RangeReportLabels& labels) {
  title_template_ = labels.title_template;
  total_time_label_ = labels.total_time_label;
  actual_days_label_ = labels.actual_days_label;
  status_days_label_ = labels.status_days_label;
  wake_anchor_days_label_ = labels.wake_anchor_days_label;
  exercise_days_label_ = labels.exercise_days_label;
  cardio_days_label_ = labels.cardio_days_label;
  anaerobic_days_label_ = labels.anaerobic_days_label;
  no_records_message_ = labels.no_records_message;
  invalid_range_message_ = labels.invalid_range_message;
  project_breakdown_label_ = labels.project_breakdown_label;
}

auto RangeBaseConfig::GetTitleTemplate() const -> const std::string& {
  return title_template_;
}

auto RangeBaseConfig::GetTotalTimeLabel() const -> const std::string& {
  return total_time_label_;
}

auto RangeBaseConfig::GetActualDaysLabel() const -> const std::string& {
  return actual_days_label_;
}

auto RangeBaseConfig::GetStatusDaysLabel() const -> const std::string& {
  return status_days_label_;
}

auto RangeBaseConfig::GetWakeAnchorDaysLabel() const -> const std::string& {
  return wake_anchor_days_label_;
}

auto RangeBaseConfig::GetExerciseDaysLabel() const -> const std::string& {
  return exercise_days_label_;
}
auto RangeBaseConfig::GetCardioDaysLabel() const -> const std::string& {
  return cardio_days_label_;
}
auto RangeBaseConfig::GetAnaerobicDaysLabel() const -> const std::string& {
  return anaerobic_days_label_;
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
