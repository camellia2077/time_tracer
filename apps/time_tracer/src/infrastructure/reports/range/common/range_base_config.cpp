// infrastructure/reports/range/common/range_base_config.cpp
#include "infrastructure/reports/range/common/range_base_config.hpp"

#include <utility>

RangeBaseConfig::RangeBaseConfig(toml::table config)
    : config_table_(std::move(config)) {
  LoadBaseConfig();
}

void RangeBaseConfig::LoadBaseConfig() {
  title_template_ = config_table_["title_template"].value_or<std::string>("");
  total_time_label_ =
      config_table_["total_time_label"].value_or<std::string>("");
  actual_days_label_ =
      config_table_["actual_days_label"].value_or<std::string>("");
  status_days_label_ =
      config_table_["status_days_label"].value_or<std::string>("Status Days");
  sleep_days_label_ =
      config_table_["sleep_days_label"].value_or<std::string>("Sleep Days");
  exercise_days_label_ =
      config_table_["exercise_days_label"].value_or<std::string>(
          "Exercise Days");
  cardio_days_label_ =
      config_table_["cardio_days_label"].value_or<std::string>("Cardio Days");
  anaerobic_days_label_ = config_table_["anaerobic_days_label"]
                              .value_or<std::string>("Anaerobic Days");
  no_records_message_ =
      config_table_["no_records_message"].value_or<std::string>("");
  invalid_range_message_ =
      config_table_["invalid_range_message"].value_or<std::string>("");
  project_breakdown_label_ =
      config_table_["project_breakdown_label"].value_or<std::string>(
          "Project Breakdown");
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
