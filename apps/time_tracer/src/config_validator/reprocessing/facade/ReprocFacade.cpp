// config_validator/reprocessing/facade/ReprocFacade.cpp
#include "ReprocFacade.hpp"
// [修改] 更新 include 路径和头文件名
#include "config_validator/reprocessing/rules/MainRule.hpp"
#include "config_validator/reprocessing/rules/MappingRule.hpp"
#include "config_validator/reprocessing/rules/DurationRule.hpp"

#include <iostream>

using json = nlohmann::json;

bool ReprocFacade::validate(
    const json& main_json,
    const json& mappings_json,
    const json& duration_rules_json
) const {
    std::string mappings_path_str, duration_rules_path_str;
    
    // [修改] 使用新的类名
    MainRule main_validator;
    if (!main_validator.validate(main_json, mappings_path_str, duration_rules_path_str)) {
        return false;
    }

    // [修改] 使用新的类名
    MappingRule mappings_validator;
    if (!mappings_validator.validate(mappings_json)) {
        return false;
    }

    // [修改] 使用新的类名
    DurationRule duration_rules_validator;
    if (!duration_rules_validator.validate(duration_rules_json)) {
        return false;
    }

    std::cout << "[Validator] All preprocessing configuration data is valid." << std::endl;
    return true;
}