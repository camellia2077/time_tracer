// infrastructure/config/validator/reports/strategies/daily/daily_typ.cpp
#include "infrastructure/config/validator/reports/strategies/daily/daily_typ.hpp"

#include <iostream>
#include <set>

auto DailyTyp::ValidateSpecificKeys(const toml::table& query_config,
                                    const std::string& file_name) const
    -> bool {
  const std::set<std::string> kDailyTypKeys = {
      "title_prefix",       "date_label",           "total_time_label",
      "status_label",       "sleep_label",          "getup_time_label",
      "remark_label",       "exercise_label",       "no_records_message",
      "statistics_label",   "all_activities_label", "activity_remark_label",
      "activity_connector", "keyword_colors",       "statistics_items"};

  for (const auto& key : kDailyTypKeys) {
    if (!query_config.contains(key)) {
      std::cerr << "[Validator] Error in " << file_name
                << ": Missing daily report key '" << key << "'." << std::endl;
      return false;
    }
  }
  return true;
}