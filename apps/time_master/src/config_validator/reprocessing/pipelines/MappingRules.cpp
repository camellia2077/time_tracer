// config_validator/reprocessing/pipelines/MappingRules.cpp
#include "MappingRules.hpp"
#include <iostream>

using json = nlohmann::json;

bool MappingRules::validate(const json& mappings_json) {
    if (!mappings_json.contains("text_mappings") || !mappings_json["text_mappings"].is_object()) {
        std::cerr << "[Validator] Error: Mappings config must contain a 'text_mappings' object." << std::endl;
        return false;
    }
    return true;
}