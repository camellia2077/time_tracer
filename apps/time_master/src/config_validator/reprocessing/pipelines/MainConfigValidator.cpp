// config_validator/reprocessing/pipelines/MainConfigValidator.cpp
#include "MainConfigValidator.hpp"
#include <iostream>
#include <set>

using json = nlohmann::json;

bool MainConfigValidator::validate(const json& main_json, std::string& out_mappings_path, std::string& out_duration_rules_path) {
    const std::set<std::string> required_keys = {
        "mappings_config_path", "duration_rules_config_path", "top_parent_mapping",
        "header_order", "remark_prefix", "wake_keywords"
    };

    for (const auto& key : required_keys) {
        if (!main_json.contains(key)) {
            std::cerr << "[Validator] Error: Main config is missing required key: '" << key << "'" << std::endl;
            return false;
        }
    }

    if (!main_json["mappings_config_path"].is_string() || !main_json["duration_rules_config_path"].is_string()) {
        std::cerr << "[Validator] Error: 'mappings_config_path' and 'duration_rules_config_path' must be strings." << std::endl;
        return false;
    }
    if (!main_json["top_parent_mapping"].is_object()) {
        std::cerr << "[Validator] Error: 'top_parent_mapping' must be a JSON object." << std::endl;
        return false;
    }
    if (!main_json["header_order"].is_array() || !main_json["wake_keywords"].is_array()) {
        std::cerr << "[Validator] Error: 'header_order' and 'wake_keywords' must be arrays." << std::endl;
        return false;
    }
    if (!main_json["remark_prefix"].is_string()) {
        std::cerr << "[Validator] Error: 'remark_prefix' must be a string." << std::endl;
        return false;
    }

    out_mappings_path = main_json["mappings_config_path"].get<std::string>();
    out_duration_rules_path = main_json["duration_rules_config_path"].get<std::string>();
    return true;
}