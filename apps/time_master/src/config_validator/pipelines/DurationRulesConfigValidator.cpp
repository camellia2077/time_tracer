// config_validator/pipelines/DurationRulesConfigValidator.cpp
#include "DurationRulesConfigValidator.hpp"
#include <iostream>

using json = nlohmann::json;

bool DurationRulesConfigValidator::validate(const json& duration_json) {
    if (!duration_json.contains("text_duration_mappings") || !duration_json["text_duration_mappings"].is_object()) {
        std::cerr << "[Validator] Error: Duration rules config must contain a 'text_duration_mappings' object." << std::endl;
        return false;
    }

    if (!duration_json.contains("duration_mappings") || !duration_json["duration_mappings"].is_object()) {
        std::cerr << "[Validator] Error: Duration rules config must contain a 'duration_mappings' object." << std::endl;
        return false;
    }

    for (const auto& [key, rules_array] : duration_json["duration_mappings"].items()) {
        if (!rules_array.is_array()) {
            std::cerr << "[Validator] Error: In duration rules, the value for key '" << key << "' must be an array." << std::endl;
            return false;
        }
        for (const auto& rule : rules_array) {
            if (!rule.is_object() || !rule.contains("less_than_minutes") || !rule.contains("value")) {
                std::cerr << "[Validator] Error: In duration rules for '" << key << "', each rule must be an object with 'less_than_minutes' and 'value' keys." << std::endl;
                return false;
            }
            if (!rule["less_than_minutes"].is_number_integer()) {
                std::cerr << "[Validator] Error: 'less_than_minutes' must be an integer for a rule in '" << key << "'." << std::endl;
                return false;
            }
            if (!rule["value"].is_string()) {
                std::cerr << "[Validator] Error: 'value' must be a string for a rule in '" << key << "'." << std::endl;
                return false;
            }
        }
    }
    return true;
}