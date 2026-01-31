// reports/range/common/range_base_config.cpp
#include "reports/range/common/range_base_config.hpp"

#include <utility>

RangeBaseConfig::RangeBaseConfig(toml::table config)
    : config_table_(std::move(config)) {
  load_base_config();
}

void RangeBaseConfig::load_base_config() {
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

auto RangeBaseConfig::get_title_template() const -> const std::string& {
  return title_template_;
}

auto RangeBaseConfig::get_total_time_label() const -> const std::string& {
  return total_time_label_;
}

auto RangeBaseConfig::get_actual_days_label() const -> const std::string& {
  return actual_days_label_;
}

auto RangeBaseConfig::get_no_records_message() const -> const std::string& {
  return no_records_message_;
}

auto RangeBaseConfig::get_invalid_range_message() const -> const std::string& {
  return invalid_range_message_;
}

auto RangeBaseConfig::get_project_breakdown_label() const
    -> const std::string& {
  return project_breakdown_label_;
}
