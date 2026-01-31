// reports/period/common/period_base_config.cpp
#include "period_base_config.hpp"

#include <stdexcept>
#include <utility>

namespace reporting {

PeriodBaseConfig::PeriodBaseConfig(toml::table config)
    : config_table_(std::move(config)) {
  load_base_config();
}

void PeriodBaseConfig::load_base_config() {
  report_title_prefix_ =
      config_table_["report_title_prefix"].value_or<std::string>(
          "Report for last");
  report_title_days_ =
      config_table_["report_title_days"].value_or<std::string>("days");
  report_title_date_separator_ =
      config_table_["report_title_date_separator"].value_or<std::string>("-");

  actual_days_label_ =
      config_table_["actual_days_label"].value_or<std::string>("");
  total_time_label_ =
      config_table_["total_time_label"].value_or<std::string>("");
  no_records_message_ =
      config_table_["no_records_message"].value_or<std::string>("");
  invalid_days_message_ =
      config_table_["invalid_days_message"].value_or<std::string>("");

  project_breakdown_label_ =
      config_table_["project_breakdown_label"].value_or<std::string>(
          "Project Breakdown");
}

auto PeriodBaseConfig::get_report_title_prefix() const -> const std::string& {
  return report_title_prefix_;
}
auto PeriodBaseConfig::get_report_title_days() const -> const std::string& {
  return report_title_days_;
}
auto PeriodBaseConfig::get_report_title_date_separator() const
    -> const std::string& {
  return report_title_date_separator_;
}
auto PeriodBaseConfig::get_actual_days_label() const -> const std::string& {
  return actual_days_label_;
}
auto PeriodBaseConfig::get_total_time_label() const -> const std::string& {
  return total_time_label_;
}
auto PeriodBaseConfig::get_no_records_message() const -> const std::string& {
  return no_records_message_;
}
auto PeriodBaseConfig::get_invalid_days_message() const -> const std::string& {
  return invalid_days_message_;
}
auto PeriodBaseConfig::get_project_breakdown_label() const
    -> const std::string& {
  return project_breakdown_label_;
}

}  // namespace reporting