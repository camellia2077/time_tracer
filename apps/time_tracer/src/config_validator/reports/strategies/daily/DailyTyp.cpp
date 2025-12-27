// config_validator/reports/strategies/daily/DailyTyp.cpp
#include "DailyTyp.hpp"
#include <iostream>
#include <set>

using json = nlohmann::json;

bool DailyTyp::validate_specific_keys(const json& query_json, const std::string& file_name) const {
    // [核心修改] 将 "no_records" 替换为 "no_records_message"
    const std::set<std::string> daily_typ_keys = {
        "title_prefix", "date_label", "total_time_label", "status_label", "sleep_label",
        "getup_time_label", "remark_label", "exercise_label", "no_records_message", "statistics_label",
        "all_activities_label", "activity_remark_label", "activity_connector", "keyword_colors",
        "statistics_items"
    };
    
    for (const auto& key : daily_typ_keys) {
        if (!query_json.contains(key)) {
            std::cerr << "[Validator] Error in " << file_name << ": Missing daily report key '" << key << "'." << std::endl;
            return false;
        }
    }
    return true;
}