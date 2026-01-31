// config/validator/reports/strategies/periodic/periodic.cpp
#include "config/validator/reports/strategies/periodic/periodic.hpp"

#include <iostream>
#include <set>

auto Periodic::validate_specific_keys(const toml::table& query_config,
                                      const std::string& file_name) const
    -> bool {
  const std::set<std::string> kPeriodicKeys = {
      "report_title_prefix", "report_title_days", "report_title_date_separator",
      "total_time_label",    "actual_days_label", "invalid_days_message",
      "no_records_message"};

  for (const auto& key : kPeriodicKeys) {
    if (!query_config.contains(key)) {
      std::cerr << "[Validator] Error in " << file_name
                << ": Missing periodic report key '" << key << "'."
                << std::endl;
      return false;
    }
  }
  return true;
}