// infrastructure/reports/monthly/common/month_base_config.cpp
#include "infrastructure/reports/monthly/common/month_base_config.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.hpp"

MonthBaseConfig::MonthBaseConfig(const TtMonthLabelsConfigV1& labels) {
  LoadBaseConfig(labels);
}

void MonthBaseConfig::LoadBaseConfig(const TtMonthLabelsConfigV1& labels) {
  report_title_ = formatter_c_string_view_utils::ToString(labels.reportTitle,
                                                          "labels.reportTitle");
  title_template_ = formatter_c_string_view_utils::ToString(
      labels.titleTemplate, "labels.titleTemplate");
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
  total_time_label_ = formatter_c_string_view_utils::ToString(
      labels.totalTimeLabel, "labels.totalTimeLabel");
  no_records_message_ = formatter_c_string_view_utils::ToString(
      labels.noRecordsMessage, "labels.noRecordsMessage");
  invalid_format_message_ = formatter_c_string_view_utils::ToString(
      labels.invalidFormatMessage, "labels.invalidFormatMessage");
  project_breakdown_label_ = formatter_c_string_view_utils::ToString(
      labels.projectBreakdownLabel, "labels.projectBreakdownLabel");
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
auto MonthBaseConfig::GetSleepDaysLabel() const -> const std::string& {
  return sleep_days_label_;
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
