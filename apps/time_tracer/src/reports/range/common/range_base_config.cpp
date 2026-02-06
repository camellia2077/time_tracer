// reports/range/common/range_base_config.cpp
#include "reports/range/common/range_base_config.hpp"

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

auto RangeBaseConfig::GetNoRecordsMessage() const -> const std::string& {
  return no_records_message_;
}

auto RangeBaseConfig::GetInvalidRangeMessage() const -> const std::string& {
  return invalid_range_message_;
}

auto RangeBaseConfig::GetProjectBreakdownLabel() const
    -> const std::string& {
  return project_breakdown_label_;
}
