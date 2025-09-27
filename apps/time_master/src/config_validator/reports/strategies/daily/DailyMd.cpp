// config_validator/reports/strategies/daily/DailyMd.cpp
#include "DailyMd.hpp"
#include <iostream>
#include <set>

using json = nlohmann::json;

bool DailyMd::validate_specific_keys(const json& query_json, const std::string& file_name) const {
    // [核心修改] 将 "no_records" 替换为 "no_records_message"
    const std::set<std::string> daily_md_keys = {
        "title_prefix", "date_label", "total_time_label", "status_label", "sleep_label", "exercise_label",
        "getup_time_label", "remark_label", "statistics_label", "all_activities_label",
        "activity_remark_label", "activity_connector", "statistics_items", "no_records_message"
    };

    for (const auto& key : daily_md_keys) {
        if (!query_json.contains(key)) {
            std::cerr << "[Validator] Error in " << file_name << ": Missing daily report key '" << key << "'." << std::endl;
            return false;
        }
    }
    return true;
}