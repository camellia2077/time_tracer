// infrastructure/config/validator/reports/strategies/daily/daily_md.cpp
#include "infrastructure/config/validator/reports/strategies/daily/daily_md.hpp"

#include <iostream>
#include <set>

auto DailyMd::ValidateSpecificKeys(const toml::table& query_config,
                                   const std::string& file_name) const -> bool {
  const std::set<std::string> kDailyMdKeys = {
      "title_prefix",          "date_label",
      "total_time_label",      "status_label",
      "sleep_label",           "exercise_label",
      "getup_time_label",      "remark_label",
      "statistics_label",      "all_activities_label",
      "activity_remark_label", "activity_connector",
      "statistics_items",      "no_records_message"};

  for (const auto& key : kDailyMdKeys) {
    if (!query_config.contains(key)) {
      std::cerr << "[Validator] Error in " << file_name
                << ": Missing daily report key '" << key << "'." << std::endl;
      return false;
    }
  }
  return true;
}