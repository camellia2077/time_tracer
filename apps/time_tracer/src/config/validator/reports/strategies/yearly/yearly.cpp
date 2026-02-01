// config/validator/reports/strategies/yearly/yearly.cpp
#include "config/validator/reports/strategies/yearly/yearly.hpp"

#include <iostream>
#include <set>

auto Yearly::validate_specific_keys(const toml::table& query_config,
                                    const std::string& file_name) const
    -> bool {
  const std::set<std::string> kYearlyKeys = {
      "title_template",     "actual_days_label",     "total_time_label",
      "no_records_message", "invalid_range_message", "project_breakdown_label"};

  for (const auto& key : kYearlyKeys) {
    if (!query_config.contains(key)) {
      std::cerr << "[Validator] Error in " << file_name
                << ": Missing yearly report key '" << key << "'." << std::endl;
      return false;
    }
  }
  return true;
}
