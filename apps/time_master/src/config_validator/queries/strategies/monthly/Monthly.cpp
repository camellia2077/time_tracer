// config_validator/queries/strategies/monthly/Monthly.cpp
#include "Monthly.hpp"
#include <iostream>
#include <set>

using json = nlohmann::json;

// [修改] 实现了 validate_specific_keys，移除了所有通用逻辑
bool Monthly::validate_specific_keys(const json& query_json, const std::string& file_name) const {
    const std::set<std::string> monthly_keys = {
        "report_title", "actual_days_label", "total_time_label", "invalid_format_message"
    };

    for (const auto& key : monthly_keys) {
        if (!query_json.contains(key)) {
            std::cerr << "[Validator] Error in " << file_name << ": Missing monthly report key '" << key << "'." << std::endl;
            return false;
        }
    }
    return true;
}