// infrastructure/config/validator/reports/strategies/weekly/weekly.cpp
#include "infrastructure/config/validator/reports/strategies/weekly/weekly.hpp"

#include <iostream>
#include <set>

auto Weekly::ValidateSpecificKeys(const toml::table& query_config,
                                  const std::string& file_name) const -> bool {
  const std::set<std::string> kWeeklyKeys = {
      "title_template",
      "actual_days_label",
      "status_days_label",
      "sleep_days_label",
      "exercise_days_label",
      "cardio_days_label",
      "anaerobic_days_label",
      "total_time_label",
      "no_records_message",
      "invalid_range_message",
      "project_breakdown_label"};

  for (const auto& key : kWeeklyKeys) {
    if (!query_config.contains(key)) {
      std::cerr << "[Validator] Error in " << file_name
                << ": Missing weekly report key '" << key << "'." << std::endl;
      return false;
    }
  }
  return true;
}
