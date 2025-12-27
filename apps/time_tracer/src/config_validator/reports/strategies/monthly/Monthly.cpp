// config_validator/reports/strategies/monthly/Monthly.cpp
#include "Monthly.hpp"
#include <iostream>
#include <set>

using json = nlohmann::json;

bool Monthly::validate_specific_keys(const json& query_json, const std::string& file_name) const {
    // [核心修改] "no_records_message" 现在是必需的
    const std::set<std::string> monthly_keys = {
        "report_title", "actual_days_label", "total_time_label", 
        "invalid_format_message", "no_records_message"
    };

    for (const auto& key : monthly_keys) {
        if (!query_json.contains(key)) {
            std::cerr << "[Validator] Error in " << file_name << ": Missing monthly report key '" << key << "'." << std::endl;
            return false;
        }
    }

    return true;
}