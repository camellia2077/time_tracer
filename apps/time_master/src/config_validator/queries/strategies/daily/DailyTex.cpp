// config_validator/queries/strategies/daily/DailyTex.cpp
#include "DailyTex.hpp"
#include <iostream>
#include <set>

using json = nlohmann::json;

// [修改] 实现了 validate_specific_keys，移除了所有通用逻辑
bool DailyTex::validate_specific_keys(const json& query_json, const std::string& file_name) const {
    const std::set<std::string> daily_tex_keys = {
        "report_title", "date_label", "total_time_label", "status_label", "sleep_label", "exercise_label",
        "getup_time_label", "remark_label", "no_records_message", "statistics_label",
        "all_activities_label", "activity_remark_label", "activity_connector", "keyword_colors",
        "statistics_items"
    };

    for (const auto& key : daily_tex_keys) {
        if (!query_json.contains(key)) {
            std::cerr << "[Validator] Error in " << file_name << ": Missing daily report key '" << key << "'." << std::endl;
            return false;
        }
    }
    return true;
}