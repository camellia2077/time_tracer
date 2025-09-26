// config_validator/reprocessing/rules/MappingRule.cpp
#include "MappingRule.hpp"
#include <iostream>

using json = nlohmann::json;

// [修改] 方法归属到 MappingRule 类
bool MappingRule::validate(const json& mappings_json) const {
    if (!mappings_json.contains("text_mappings") || !mappings_json["text_mappings"].is_object()) {
        std::cerr << "[Validator] Error: Mappings config must contain a 'text_mappings' object." << std::endl;
        return false;
    }
    return true;
}