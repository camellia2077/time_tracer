// infrastructure/reports/range/common/range_base_config.cpp
#include "infrastructure/reports/range/common/range_base_config.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.hpp"

RangeBaseConfig::RangeBaseConfig(const TtRangeLabelsConfigV1& labels) {
  LoadBaseConfig(labels);
}

void RangeBaseConfig::LoadBaseConfig(const TtRangeLabelsConfigV1& labels) {
  title_template_ = formatter_c_string_view_utils::ToString(
      labels.titleTemplate, "labels.titleTemplate");
  total_time_label_ = formatter_c_string_view_utils::ToString(
      labels.totalTimeLabel, "labels.totalTimeLabel");
  actual_days_label_ = formatter_c_string_view_utils::ToString(
      labels.actualDaysLabel, "labels.actualDaysLabel");
  status_days_label_ = formatter_c_string_view_utils::ToString(
      labels.statusDaysLabel, "labels.statusDaysLabel");
  sleep_days_label_ = formatter_c_string_view_utils::ToString(
      labels.sleepDaysLabel, "labels.sleepDaysLabel");
  exercise_days_label_ = formatter_c_string_view_utils::ToString(
      labels.exerciseDaysLabel, "labels.exerciseDaysLabel");
  cardio_days_label_ = formatter_c_string_view_utils::ToString(
      labels.cardioDaysLabel, "labels.cardioDaysLabel");
  anaerobic_days_label_ = formatter_c_string_view_utils::ToString(
      labels.anaerobicDaysLabel, "labels.anaerobicDaysLabel");
  no_records_message_ = formatter_c_string_view_utils::ToString(
      labels.noRecordsMessage, "labels.noRecordsMessage");
  invalid_range_message_ = formatter_c_string_view_utils::ToString(
      labels.invalidRangeMessage, "labels.invalidRangeMessage");
  project_breakdown_label_ = formatter_c_string_view_utils::ToString(
      labels.projectBreakdownLabel, "labels.projectBreakdownLabel");
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

auto RangeBaseConfig::GetSleepDaysLabel() const -> const std::string& {
  return sleep_days_label_;
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
