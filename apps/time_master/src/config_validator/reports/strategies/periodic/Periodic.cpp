// config_validator/reports/strategies/periodic/Periodic.cpp
#include "Periodic.hpp"
#include <iostream>
#include <set>

using json = nlohmann::json;

bool Periodic::validate_specific_keys(const json& query_json, const std::string& file_name) const {
    // [核心修改] "no_records_message" 现在是必需的
    const std::set<std::string> periodic_keys = {
        "report_title_prefix", "report_title_days", "report_title_date_separator",
        "total_time_label", "actual_days_label", "invalid_days_message", "no_records_message"
    };

    for (const auto& key : periodic_keys) {
        if (!query_json.contains(key)) {
            std::cerr << "[Validator] Error in " << file_name << ": Missing periodic report key '" << key << "'." << std::endl;
            return false;
        }
    }
    return true;
}