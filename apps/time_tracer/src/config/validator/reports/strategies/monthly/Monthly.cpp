// config/validator/reports/strategies/monthly/monthly.cpp
#include "monthly.hpp"
#include <iostream>
#include <set>

bool Monthly::validate_specific_keys(const toml::table& query_config, const std::string& file_name) const {
    const std::set<std::string> monthly_keys = {
        "report_title", "actual_days_label", "total_time_label", 
        "no_records_message"
    };

    for (const auto& key : monthly_keys) {
        if (!query_config.contains(key)) {
            std::cerr << "[Validator] Error in " << file_name << ": Missing monthly report key '" << key << "'." << std::endl;
            return false;
        }
    }
    return true;
}