// config/validator/reports/strategies/daily/DailyTyp.cpp
#include "DailyTyp.hpp"
#include <iostream>
#include <set>

bool DailyTyp::validate_specific_keys(const toml::table& query_config, const std::string& file_name) const {
    const std::set<std::string> daily_typ_keys = {
        "title_prefix", "date_label", "total_time_label", "status_label", "sleep_label",
        "getup_time_label", "remark_label", "exercise_label", "no_records_message", "statistics_label",
        "all_activities_label", "activity_remark_label", "activity_connector", "keyword_colors",
        "statistics_items"
    };
    
    for (const auto& key : daily_typ_keys) {
        if (!query_config.contains(key)) {
            std::cerr << "[Validator] Error in " << file_name << ": Missing daily report key '" << key << "'." << std::endl;
            return false;
        }
    }
    return true;
}