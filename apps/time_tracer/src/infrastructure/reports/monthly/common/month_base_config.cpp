// infrastructure/reports/monthly/common/month_base_config.cpp
#include "infrastructure/reports/monthly/common/month_base_config.hpp"

#include <stdexcept>
#include <utility>

MonthBaseConfig::MonthBaseConfig(toml::table config)
    : config_table_(std::move(config)) {
  LoadBaseConfig();
}

void MonthBaseConfig::LoadBaseConfig() {
  // 使用 toml++ API 获取值
  // value_or 对于必填项如果不提供默认值会比较麻烦，这里假设配置文件是完整的
  // 提供默认空字符串以避免崩溃
  report_title_ = config_table_["report_title"].value_or<std::string>("");
  title_template_ = config_table_["title_template"].value_or<std::string>(
      std::string(report_title_));
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
  total_time_label_ =
      config_table_["total_time_label"].value_or<std::string>("");
  invalid_format_message_ =
      config_table_["invalid_format_message"].value_or<std::string>("");
  no_records_message_ =
      config_table_["no_records_message"].value_or<std::string>("");
  project_breakdown_label_ =
      config_table_["project_breakdown_label"].value_or<std::string>(
          "Project Breakdown");
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
