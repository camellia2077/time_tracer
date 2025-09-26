// config_validator/reprocessing/facade/ReprocFacade.cpp
#include "ReprocFacade.hpp"
#include "config_validator/reprocessing/pipelines/MainRules.hpp"
#include "config_validator/reprocessing/pipelines/MappingRules.hpp"
#include "config_validator/reprocessing/pipelines/DurationRules.hpp"

#include <iostream>

using json = nlohmann::json;

bool ReprocFacade::validate(
    const json& main_json,
    const json& mappings_json,
    const json& duration_rules_json
) const {
    std::string mappings_path_str, duration_rules_path_str;
    MainRules main_validator;
    if (!main_validator.validate(main_json, mappings_path_str, duration_rules_path_str)) {
        return false;
    }

    MappingRules mappings_validator;
    if (!mappings_validator.validate(mappings_json)) {
        return false;
    }

    DurationRules duration_rules_validator;
    if (!duration_rules_validator.validate(duration_rules_json)) {
        return false;
    }

    std::cout << "[Validator] All preprocessing configuration data is valid." << std::endl;
    return true;
}