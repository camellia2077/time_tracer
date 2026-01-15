// config/validator/converter/facade/ConverterFacade.cpp
#include "ConverterFacade.hpp"
#include "config/validator/converter/rules/MainRule.hpp"
#include "config/validator/converter/rules/MappingRule.hpp"
#include "config/validator/converter/rules/DurationRule.hpp"

#include <iostream>

bool ConverterFacade::validate(
    const toml::table& main_tbl,
    const toml::table& mappings_tbl,
    const toml::table& duration_rules_tbl
) const {
    std::string mappings_path_str, duration_rules_path_str;
    
    MainRule main_validator;
    if (!main_validator.validate(main_tbl, mappings_path_str, duration_rules_path_str)) {
        return false;
    }

    MappingRule mappings_validator;
    if (!mappings_validator.validate(mappings_tbl)) {
        return false;
    }

    DurationRule duration_rules_validator;
    if (!duration_rules_validator.validate(duration_rules_tbl)) {
        return false;
    }

    std::cout << "[Validator] All preprocessing configuration data (TOML) is valid." << std::endl;
    return true;
}