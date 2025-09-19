// config_validator/reprocessing/facade/ReprocessingValidatorFacade.cpp
#include "ReprocessingValidatorFacade.hpp"
#include "../pipelines/MainConfigValidator.hpp"
#include "../pipelines/MappingsConfigValidator.hpp"
#include "../pipelines/DurationRulesConfigValidator.hpp"

#include <iostream>

using json = nlohmann::json;

bool ReprocessingValidatorFacade::validate(
    const json& main_json,
    const json& mappings_json,
    const json& duration_rules_json
) const {
    std::string mappings_path_str, duration_rules_path_str;
    MainConfigValidator main_validator;
    if (!main_validator.validate(main_json, mappings_path_str, duration_rules_path_str)) {
        return false;
    }

    MappingsConfigValidator mappings_validator;
    if (!mappings_validator.validate(mappings_json)) {
        return false;
    }

    DurationRulesConfigValidator duration_rules_validator;
    if (!duration_rules_validator.validate(duration_rules_json)) {
        return false;
    }

    std::cout << "[Validator] All preprocessing configuration data is valid." << std::endl;
    return true;
}